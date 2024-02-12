#include "DescParser.h"
#include "paklib/PakInterface.h"

using namespace Sexy;

DescParser::DescParser() { mCmdSep = CMDSEP_SEMICOLON; }

DescParser::~DescParser() {}

bool DescParser::Error(const std::string &theError) {
    mError = theError;
    return false;
}

DataElement *DescParser::Dereference(const std::string &theString) {
    std::string aDefineName = StringToUpper(theString);

    auto anItr = mDefineMap.find(aDefineName);
    if (anItr != mDefineMap.end()) return anItr->second;
    else return nullptr;
}

bool DescParser::IsImmediate(const std::string &theString) {
    return (
        ((theString[0] >= '0') && (theString[0] <= '9')) || (theString[0] == '-') || (theString[0] == '+') ||
        (theString[0] == '\'') || (theString[0] == '"')
    );
}

std::string DescParser::Unquote(const std::string &theQuotedString) {
    if ((theQuotedString[0] == '\'') || (theQuotedString[0] == '"')) {
        char aQuoteChar = theQuotedString[0];
        std::string aLiteralString;
        bool lastWasQuote = false;

        for (ulong i = 0; i < theQuotedString.length(); i++) {
            if (theQuotedString[i] == aQuoteChar) {
                if (lastWasQuote) aLiteralString += aQuoteChar;

                lastWasQuote = true;
            } else {
                aLiteralString += theQuotedString[i];
                lastWasQuote = false;
            }
        }

        return aLiteralString;
    } else return theQuotedString;
}

bool DescParser::GetValues(ListDataElement *theSource, ListDataElement *theValues) {
    theValues->mElementVector.clear();

    for (ulong aSourceNum = 0; aSourceNum < theSource->mElementVector.size(); aSourceNum++) {
        if (theSource->mElementVector[aSourceNum]->mIsList) {
            auto aChildList = new ListDataElement();
            theValues->mElementVector.push_back(aChildList);

            if (!GetValues(static_cast<ListDataElement *>(theSource->mElementVector[aSourceNum]), aChildList))
                return false;
        } else {
            std::string aString = static_cast<SingleDataElement *>(theSource->mElementVector[aSourceNum])->mString;

            if (aString.length() > 0) {
                if ((aString[0] == '\'') || (aString[0] == '"')) {
                    auto aChildData = new SingleDataElement(Unquote(aString));
                    theValues->mElementVector.push_back(aChildData);
                } else if (IsImmediate(aString)) {
                    theValues->mElementVector.push_back(new SingleDataElement(aString));
                } else {
                    std::string aDefineName = StringToUpper(aString);

                    auto anItr = mDefineMap.find(aDefineName);

                    if (anItr == mDefineMap.end()) {
                        Error("Unable to Dereference \"" + aString + "\"");
                        return false;
                    }

                    theValues->mElementVector.push_back(anItr->second->Duplicate());
                }
            }
        }
    }

    return true;
}

std::string DescParser::DataElementToString(DataElement *theDataElement) {
    if (theDataElement->mIsList) {
        auto aListDataElement = static_cast<ListDataElement *>(theDataElement);

        std::string aString = "(";

        for (ulong i = 0; i < aListDataElement->mElementVector.size(); i++) {
            if (i != 0) aString += ", ";

            aString += DataElementToString(aListDataElement->mElementVector[i]);
        }

        aString += ")";

        return aString;
    } else {
        auto aSingleDataElement = static_cast<SingleDataElement *>(theDataElement);
        return aSingleDataElement->mString;
    }
}

bool DescParser::DataToString(DataElement *theSource, std::string *theString) {
    *theString = "";

    if (theSource->mIsList) return false;

    std::string aDefName = static_cast<SingleDataElement *>(theSource)->mString;

    DataElement *aDataElement = Dereference(aDefName);

    if (aDataElement != nullptr) {
        if (aDataElement->mIsList) return false;

        *theString = Unquote(static_cast<SingleDataElement *>(aDataElement)->mString);
    } else *theString = Unquote(aDefName);

    return true;
}

bool DescParser::DataToInt(DataElement *theSource, int *theInt) {
    *theInt = 0;

    std::string aTempString;
    if (!DataToString(theSource, &aTempString)) return false;

    if (!StringToInt(aTempString, theInt)) return false;

    return true;
}

bool DescParser::DataToStringVector(DataElement *theSource, StringVector *theStringVector) {
    theStringVector->clear();

    auto aStaticValues = ListDataElement();
    ListDataElement *aValues;

    if (theSource->mIsList) {
        if (!GetValues(static_cast<ListDataElement *>(theSource), &aStaticValues)) return false;

        aValues = &aStaticValues;
    } else {
        std::string aDefName = static_cast<SingleDataElement *>(theSource)->mString;

        DataElement *aDataElement = Dereference(aDefName);

        if (aDataElement == nullptr) {
            Error("Unable to Dereference \"" + aDefName + "\"");
            return false;
        }

        if (!aDataElement->mIsList) return false;

        aValues = static_cast<ListDataElement *>(aDataElement);
    }

    for (ulong i = 0; i < aValues->mElementVector.size(); i++) {
        if (aValues->mElementVector[i]->mIsList) {
            theStringVector->clear();
            return false;
        }

        auto aSingleDataElement = static_cast<SingleDataElement *>(aValues->mElementVector[i]);

        theStringVector->push_back(aSingleDataElement->mString);
    }

    return true;
}

bool DescParser::DataToList(DataElement *theSource, ListDataElement *theValues) {
    if (theSource->mIsList) {
        return GetValues(static_cast<ListDataElement *>(theSource), theValues);
    }

    DataElement *aDataElement = Dereference(static_cast<SingleDataElement *>(theSource)->mString);

    if ((aDataElement == nullptr) || (!aDataElement->mIsList)) return false;

    auto aListElement = static_cast<ListDataElement *>(aDataElement);

    *theValues = *aListElement;

    return true;
}

bool DescParser::DataToIntVector(DataElement *theSource, IntVector *theIntVector) {
    theIntVector->clear();

    auto aStringVector = StringVector();
    if (!DataToStringVector(theSource, &aStringVector)) return false;

    for (ulong i = 0; i < aStringVector.size(); i++) {
        int aIntVal;
        if (!StringToInt(aStringVector[i], &aIntVal)) return false;

        theIntVector->push_back(aIntVal);
    }

    return true;
}

bool DescParser::DataToDoubleVector(DataElement *theSource, DoubleVector *theDoubleVector) {
    theDoubleVector->clear();

    StringVector aStringVector;
    if (!DataToStringVector(theSource, &aStringVector)) return false;

    for (ulong i = 0; i < aStringVector.size(); i++) {
        double aDoubleVal;
        if (!StringToDouble(aStringVector[i], &aDoubleVal)) return false;

        theDoubleVector->push_back(aDoubleVal);
    }

    return true;
}

bool DescParser::ParseToList(
    const std::string &theString, ListDataElement *theList, bool expectListEnd, int *theStringPos
) {
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;
    bool escaped = false;

    SingleDataElement *aCurSingleDataElement = nullptr;

    int aStringPos = 0;

    if (theStringPos == nullptr) theStringPos = &aStringPos;

    while (*theStringPos < static_cast<int>(theString.length())) {
        bool addSingleChar = false;
        char aChar = theString[(*theStringPos)++];

        bool isSeperator = (aChar == ' ') || (aChar == '\t') || (aChar == '\n') || (aChar == ',');

        if (escaped) {
            addSingleChar = true;
            escaped = false;
        } else {
            if ((aChar == '\'') && (!inDoubleQuotes)) inSingleQuotes = !inSingleQuotes;
            else if ((aChar == '"') && (!inSingleQuotes)) inDoubleQuotes = !inDoubleQuotes;

            if (aChar == '\\') {
                escaped = true;
            } else if ((!inSingleQuotes) && (!inDoubleQuotes)) {
                if (aChar == ')') {
                    if (expectListEnd) return true;
                    else {
                        Error("Unexpected List End");
                        return false;
                    }
                } else if (aChar == '(') {
                    if (aCurSingleDataElement != nullptr) {
                        Error("Unexpected List Start");
                        return false;
                    } else {
                        auto aChildList = new ListDataElement();

                        if (!ParseToList(theString, aChildList, true, theStringPos)) return false;

                        theList->mElementVector.push_back(aChildList);
                    }
                } else if (isSeperator) {
                    if (aCurSingleDataElement != nullptr) aCurSingleDataElement = nullptr;
                } else addSingleChar = true;
            } else addSingleChar = true;
        }

        if (addSingleChar) {
            if (aCurSingleDataElement == nullptr) {
                aCurSingleDataElement = new SingleDataElement();
                theList->mElementVector.push_back(aCurSingleDataElement);
            }

            aCurSingleDataElement->mString += aChar;
        }
    }

    if (inSingleQuotes) {
        Error("Unterminated Single Quotes");
        return false;
    }

    if (inDoubleQuotes) {
        Error("Unterminated Double Quotes");
        return false;
    }

    if (expectListEnd) {
        Error("Unterminated List");
        return false;
    }

    return true;
}

bool DescParser::ParseDescriptorLine(const std::string &theDescriptorLine) {
    ListDataElement aParams;
    if (!ParseToList(theDescriptorLine, &aParams, false, nullptr)) return false;

    if (aParams.mElementVector.size() > 0) {
        if (aParams.mElementVector[0]->mIsList) {
            Error("Missing Command");
            return false;
        }

        if (!HandleCommand(aParams)) return false;
    }

    return true;
}

bool DescParser::LoadDescriptor(const std::string &theFileName) {
    mCurrentLineNum = 0;
    int aLineCount = 0;
    bool hasErrors = false;

    // Apparently VC6 doesn't have a clear() function for basic_strings
    mError.clear();
    // mError.erase();
    // mError.erase(mError.begin());

    PFILE *aStream = p_fopen(theFileName.c_str(), "r");
    if (aStream == nullptr) return false;

    char aBuffChar = 0;

    while (!p_feof(aStream)) {
        int aChar;

        bool skipLine = false;
        bool atLineStart = true;
        bool inSingleQuotes = false;
        bool inDoubleQuotes = false;
        bool escaped = false;
        bool isIndented = false;

        for (;;) {
            if (aBuffChar != 0) {
                aChar = aBuffChar;
                aBuffChar = 0;
            } else {
                aChar = p_fgetc(aStream);
                if (aChar == EOF) break;
            }

            if (aChar != '\r') {
                if (aChar == '\n') aLineCount++;

                if (((aChar == ' ') || (aChar == '\t')) && (atLineStart)) isIndented = true;

                if ((!atLineStart) || ((aChar != ' ') && (aChar != '\t') && (aChar != '\n'))) {
                    if (atLineStart) {
                        if ((mCmdSep & CMDSEP_NO_INDENT) && (!isIndented) && (mCurrentLine.size() > 0)) {
                            // Start a new non-indented line
                            aBuffChar = aChar;
                            break;
                        }

                        if (aChar == '#') skipLine = true;

                        atLineStart = false;
                    }

                    if (aChar == '\n') {
                        isIndented = false;
                        atLineStart = true;
                    }

                    if ((aChar == '\n') && (skipLine)) {
                        skipLine = false;
                    } else if (!skipLine) {
                        if (aChar == '\\' && (inSingleQuotes || inDoubleQuotes) && !escaped) escaped = true;
                        else {
                            if ((aChar == '\'') && (!inDoubleQuotes) && (!escaped)) inSingleQuotes = !inSingleQuotes;

                            if ((aChar == '"') && (!inSingleQuotes) && (!escaped)) inDoubleQuotes = !inDoubleQuotes;

                            if ((aChar == ';') && (mCmdSep & CMDSEP_SEMICOLON) && (!inSingleQuotes) &&
                                (!inDoubleQuotes))
                                break;

                            if (escaped) // stay escaped for when this is actually parsed
                            {
                                mCurrentLine += '\\';
                                escaped = false;
                            }

                            if (mCurrentLine.size() == 0) mCurrentLineNum = aLineCount + 1;

                            mCurrentLine += aChar;
                        }
                    }
                }
            }
        }

        if (mCurrentLine.length() > 0) {
            if (!ParseDescriptorLine(mCurrentLine)) {
                hasErrors = true;
                break;
            }

            // Apparently VC6 doesn't have a clear() function for basic_strings
            // mCurrentLine.clear();
            mCurrentLine.clear();
        }
    }

    // Apparently VC6 doesn't have a clear() function for basic_strings
    mCurrentLine.clear();
    // mCurrentLine.erase();
    mCurrentLineNum = 0;

    p_fclose(aStream);
    return !hasErrors;
}
