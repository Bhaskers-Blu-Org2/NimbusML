// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "stdafx.h"
#include "DataViewInterop.h"
#include "ManagedInterop.h"


DataSourceBlock::DataSourceBlock(bp::dict& data)
{
    // Assert that this class doesn't have a vtable.
    assert(offsetof(DataSourceBlock, ccol) == 0);

    CxInt64 llTotalNumRows = -1;
    assert(data.contains(PYTHON_DATA_KEY_INFO));
    bp::dict varInfo = bp::extract<bp::dict>(data[PYTHON_DATA_KEY_INFO]);

    assert(data.contains(PYTHON_DATA_COL_TYPES));
    bp::list colTypes = bp::extract_or_cast<bp::list>(data[PYTHON_DATA_COL_TYPES]);
    CxInt64 dataframeColCount = -1;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (!bp::isinstance<bp::str>(it->first))
            continue;

        // Function PyUnicode_AS_DATA is deprecated as getting the expected unicode
        // requires the creation of a new object. We keep a copy of the string.
        // See https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_AS_DATA.
        // pybind11 uses a bytes representation of the string and then call
        // PyUnicode_AsEncodedString which creates a new reference.
        bp::str skey = bp::cast<bp::str>(it->first.ptr());
        std::string string_name = (std::string)skey;
        const char* name = string_name.c_str();
        if (strcmp(name, PYTHON_DATA_KEY_INFO) == 0 || strcmp(name, PYTHON_DATA_COL_TYPES) == 0)
            continue;

        bp::object value = bp::cast<bp::object>(it->second);
        // now it should be a column names
        std::string colName = bp::extract_or_cast<std::string>(it->first);
        dataframeColCount++;
        const char* tp = (char*)PyUnicode_DATA(colTypes[dataframeColCount].ptr());
        ML_PY_TYPE_MAP_ENUM colType = static_cast<ML_PY_TYPE_MAP_ENUM>(tp[0]);

        BYTE kind;
        void *pgetter;
        bool isKey = false;
        bool isNumeric = false;
        bool isText = false;
        CxInt64 vecCard = -1;
        // Numeric or bool values.
        if (bp::isinstance<numpy_array>(value))
        {
            isNumeric = true;
            np::ndarray val = bp::extract<np::ndarray>(value);
            switch (colType)
            {
            case (ML_PY_BOOL):
                kind = BL;
                pgetter = (void*)&GetBL;
                break;
            case (ML_PY_BOOL64):
                kind = BL;
                pgetter = (void*)&GetBL64;
                break;
            case (ML_PY_UINT8):
                kind = U1;
                pgetter = (void*)&GetU1;
                break;
            case (ML_PY_UINT16):
                kind = U2;
                pgetter = (void*)&GetU2;
                break;
            case (ML_PY_UINT32):
                kind = U4;
                pgetter = (void*)&GetU4;
                break;
            case (ML_PY_UINT64):
                kind = U8;
                pgetter = (void*)&GetU8;
                break;
            case (ML_PY_INT8):
                kind = I1;
                pgetter = (void*)&GetI1;
                break;
            case (ML_PY_INT16):
                kind = I2;
                pgetter = (void*)&GetI2;
                break;
            case (ML_PY_INT32):
                kind = I4;
                pgetter = (void*)&GetI4;
                break;
            case (ML_PY_INT64):
                kind = I8;
                pgetter = (void*)&GetI8;
                break;
            case (ML_PY_FLOAT16):
                // What to do with numpy.float16 ?
                throw std::invalid_argument("numpy.float16 data type is not supported");
            case (ML_PY_FLOAT32):
                kind = R4;
                pgetter = (void*)&GetR4;
                break;
            case (ML_PY_FLOAT64):
                kind = R8;
                pgetter = (void*)&GetR8;
                break;
            case (ML_PY_DATETIME):
                kind = DT;
                pgetter = (void*)&GetI8;
                break;
            default:
                throw std::invalid_argument("column " + colName + " has unsupported type");
            }
            const char *data = val.data();
            this->_vdata.push_back(data);

            assert(this->_mpnum.size() == dataframeColCount);
            this->_mpnum.push_back(_vdata.size() - 1);
            if (llTotalNumRows == -1)
                llTotalNumRows = val.shape(0);
            else
                assert(llTotalNumRows == val.shape(0));
        }
        // Text or key values.
        else if (bp::isinstance<bp::list>(value))
        {
            bp::list list = bp::extract_or_cast<bp::list>(value);

            // Key values.
            switch (colType)
            {
            case (ML_PY_CAT):
                if (varInfo.contains(bp::str(colName)))
                {
                    isKey = true;
                    assert(bp::isinstance<bp::list>(varInfo[bp::str(colName)]));
                    bp::list keyNames = bp::extract_or_cast<bp::list>(varInfo[bp::str(colName)]);

                    kind = U4;
                    pgetter = (void*)GetKeyInt;

                    // TODO: Handle vectors.
                    this->_vkeyCard.push_back(len(keyNames));
                    //this->_vvecCard.push_back(vecCard);
                    this->_vkeydata.push_back(list);
                    this->_vkeynames.push_back(keyNames);

                    assert(this->_mpkey.size() == dataframeColCount);
                    this->_mpkey.push_back(_vkeydata.size() - 1);
                    if (llTotalNumRows == -1)
                        llTotalNumRows = len(list);
                    else
                        assert(llTotalNumRows == len(list));
                }
                else
                    continue;
                break;
                // Text values.
            case (ML_PY_TEXT):
            case (ML_PY_UNICODE):
                isText = true;
                kind = TX;
                if (colType == ML_PY_TEXT)
                    pgetter = (void*)GetTX;
                else // colType is "unicode"
                     // in python 2.7 strings can be passed as unicode bytestring (NOT the same as UTF8 encoded strings)
                    pgetter = (void*)GetUnicodeTX;

                // TODO: Handle vectors.
                //this->_vvecCard.push_back(vecCard);
                this->_vtextdata.push_back(list);

                assert(this->_mptxt.size() == dataframeColCount);
                this->_mptxt.push_back(_vtextdata.size() - 1);
                if (llTotalNumRows == -1)
                    llTotalNumRows = len(list);
                else
                    assert(llTotalNumRows == len(list));
                break;
            default:
                throw std::invalid_argument("column " + colName + " has unsupported type");
            }
        }
        // A sparse vector.
        else if (bp::isinstance<bp::dict>(value))
        {
            bp::dict sparse = bp::extract<bp::dict>(value);
            np::ndarray indices = bp::extract<np::ndarray>(sparse["indices"]);
            _sparseIndices = (int*)indices.data();
            np::ndarray indptr = bp::extract<np::ndarray>(sparse["indptr"]);
            _indPtr = (int*)indptr.data();

            np::ndarray values = bp::extract<np::ndarray>(sparse["values"]);
            _sparseValues = values.data();
            switch (colType)
            {
            case (ML_PY_BOOL):
                kind = BL;
                pgetter = (void*)&GetBLVector;
                break;
            case (ML_PY_UINT8):
                kind = U1;
                pgetter = (void*)&GetU1Vector;
                break;
            case (ML_PY_UINT16):
                kind = U2;
                pgetter = (void*)&GetU2Vector;
                break;
            case (ML_PY_UINT32):
                kind = U4;
                pgetter = (void*)&GetU4Vector;
                break;
            case (ML_PY_UINT64):
                kind = U8;
                pgetter = (void*)&GetU8Vector;
                break;
            case (ML_PY_INT8):
                kind = I1;
                pgetter = (void*)&GetI1Vector;
                break;
            case (ML_PY_INT16):
                kind = I2;
                pgetter = (void*)&GetI2Vector;
                break;
            case (ML_PY_INT32):
                kind = I4;
                pgetter = (void*)&GetI4Vector;
                break;
            case (ML_PY_INT64):
                kind = I8;
                pgetter = (void*)&GetI8Vector;
                break;
            case (ML_PY_FLOAT16):
                throw std::invalid_argument("numpy.float16 data type is not supported in sparse data");
            case (ML_PY_FLOAT32):
                kind = R4;
                pgetter = (void*)&GetR4Vector;
                break;
            case (ML_PY_FLOAT64):
                kind = R8;
                pgetter = (void*)&GetR8Vector;
                break;
            default:
                throw std::invalid_argument("column " + colName + " has unsupported type");
            }
            vecCard = bp::extract<int>(sparse["colCount"]);
            name = (char*)"Data";

            if (llTotalNumRows == -1)
                llTotalNumRows = len(indptr) - 1;
            else
                assert(llTotalNumRows == len(indptr) - 1);
        }
        else
            throw std::invalid_argument("unsupported data type provided");

        this->_vgetter.push_back(pgetter);
        this->_vname.push_back(name);
        this->_vkind.push_back(kind);
        _vvecCard.push_back(vecCard);

        if (!isNumeric)
        {
            assert(this->_mpnum.size() == dataframeColCount);
            this->_mpnum.push_back(-1);
        }
        if (!isKey)
        {
            assert(this->_mpkey.size() == dataframeColCount);
            this->_mpkey.push_back(-1);
            this->_vkeyCard.push_back(-1);
        }
        if (!isText)
        {
            assert(this->_mptxt.size() == dataframeColCount);
            this->_mptxt.push_back(-1);
        }
    }

    assert(_vname.size() <= (size_t)(dataframeColCount + 1));

    this->crow = llTotalNumRows;
    this->ccol = this->_vname.size();
    this->getLabels = &GetKeyNames;

    assert(this->ccol == this->_vkind.size());
    assert(this->ccol == this->_vkeyCard.size());
    assert(this->ccol == this->_vgetter.size());

    if (this->ccol > 0)
    {
        this->names = &this->_vname[0];
        this->kinds = &this->_vkind[0];
        this->keyCards = &this->_vkeyCard[0];
        this->vecCards = &this->_vvecCard[0];
        this->getters = &this->_vgetter[0];
    }
    else
    {
        this->names = nullptr;
        this->kinds = nullptr;
        this->keyCards = nullptr;
        this->vecCards = nullptr;
        this->getters = nullptr;
    }
}

DataSourceBlock::~DataSourceBlock()
{

#if _MSC_VER
    for (std::vector<char*>::iterator it = this->_vtextdata_cache.begin(); it != this->_vtextdata_cache.end(); ++it) {
        char* tmp = *it;
        if (tmp != NULL)
            free(tmp);
    }
#endif

    FillDead(this->ccol);
    FillDead(this->crow);

    FillDead(this->names);
    FillDead(this->kinds);
    FillDead(this->keyCards);
    FillDead(this->vecCards);
    FillDead(this->getters);
    FillDead(this->getLabels);
}



