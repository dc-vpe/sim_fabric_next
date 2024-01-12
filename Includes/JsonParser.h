//
// Created by krw10 on 12/7/2023.
//

#ifndef DSL_CPP_JSON_PARSER_H
#define DSL_CPP_JSON_PARSER_H

/*
 * Portions of this class have been developed with information obtained by
 * reviewing SimpleJSON library, licensed under MIT, see below:
 *
 * SimpleJSON Library - http://mjpa.in/json
 *
 * Copyright (C) 2010 Mike Anchor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "U8String.h"
#include "DslValue.h"
#include "ParseData.h"
#include "Queue.h"
#include <cmath>
#include "Stack.h"

/// \desc Mini json processor, translates a collection into its json string representation.
class JsonParser
{
public:
    DslValue *root;
    DslValue *current;
    int64_t rootIndex; //Used for empty objects so they have unique names.
    U8String *rootName;
    List<U8String *>jsonParseErrors;
    Queue<DslValue *> queueValues;
    Stack<DslValue *>nodes;


    JsonParser()
    {
        root = nullptr;
        current = nullptr;
        rootIndex = 1;
        rootName = new U8String("root");
        jsonParseErrors = {};
    }

    ~JsonParser()
    {
        delete rootName;
    }

    /// \desc Adds a new collection, updating the node list and current collection.
    /// \param name Name of the collection to add.
    void add_variable(U8String *name)
    {
        auto *dslValue = new DslValue(name);
        dslValue->opcode = DEF;
        dslValue->type = COLLECTION;
        dslValue->operand = program.Count();

        if ( root == nullptr )
        {
            dslValue->jsonKey.CopyFrom(name);
            root = dslValue;
            current = root;
        }
        else
        {
            dslValue->jsonKey.CopyFrom(new U8String(name));
            current->indexes.Set(name, dslValue);
            current = dslValue;
        }
    }

    /// \desc Parsers the UTF8 json formatted text returning a DSL collection.
    /// \param json Pointer to a U8String
    DslValue *From(U8String *rName, U8String *jsonText)
    {
        auto *buffer = (u8chr *)calloc(jsonText->Count()+1,  sizeof(u8chr));

        jsonText->GetBuffer(buffer);

        rootIndex = 0;
        rootName->CopyFrom(rName);

        u8chr *data = buffer;
        skip_white_space((const u8chr **)&data);

        jsonParseErrors.Clear();

        Parse((const u8chr **)&data);
        free(buffer);

        if ( jsonParseErrors.Count() > 0 )
        {
            auto *dslValue = new DslValue();
            dslValue->type = ERROR_TOKEN;
            dslValue->sValue.CopyFromCString("Json format is invalid, Json format must be compatible with RFC 7195\n");
            for(int64_t ii=0; ii<jsonParseErrors.Count(); ++ii)
            {
                dslValue->sValue.push_back(jsonParseErrors.get(ii));
                delete jsonParseErrors.get(ii);
            }

            return dslValue;
        }
#ifdef ADD_DEBUG_INFO
// debug code dumps out the values queued by the parser.
//      for(int ii=0; ii<queueValues.Count(); ++ii)
//      {
//          DslValue *d = queueValues.Peek(ii);
//          d->Print(true);
//          printf(" ");
//      }
#endif
        //Create the internal collection structure.
        while( !queueValues.IsEmpty() )
        {
            DslValue *v = queueValues.Dequeue();
            if ( v->type == CLOSE_BLOCK || v->type == CLOSE_BRACE )
            {
                if( nodes.top() > 0 )
                {
                    current = nodes.pop_back();
                }
                else
                {
                    current = root;
                }
                continue;
            }
            if ( v->type == OPEN_BLOCK || v->type == OPEN_BRACE )
            {
                if( root == nullptr )
                {
                    add_variable(rootName);
                    DslValue *prev = current;
                    add_variable(&v->jsonKey);
                    v->type = COLLECTION;
                    prev->indexes.Set(new U8String(v->jsonKey), current);
                }
                else
                {
                    v->type = COLLECTION;
                    DslValue *prev = current;
                    add_variable(&v->jsonKey);
                    prev->indexes.Set(new U8String(v->jsonKey), current);
                    nodes.push_back(prev);
                }
            }
            else
            {
                if ( root == nullptr )
                {
                    add_variable(rootName);
                    DslValue *prev = current;
                }
                current->indexes.Set(new U8String(v->jsonKey), new DslValue(v));
            }
        }

        return root;
    }

    /// \desc check that the string has at least n characters left.
    static inline bool check_at_least(const u8chr *s, size_t n)
    {
        if (s == nullptr)
            return false;

        const u8chr *save = s;
        while (n-- > 0)
        {
            if (*(save++) == 0)
            {
                return false;
            }
        }

        return true;
    }

    static inline bool json_compare(const u8chr *s, const char *c, int64_t len)
    {
        for(int64_t ii=0; ii<len; ++ii)
        {
            if ( s[ii] != c[ii] )
            {
                return false;
            }
        }

        return true;
    }

    /// \desc Checks if the UTF8 character is a whitespace character.
    static bool is_whitespace(u8chr ch)
    {
        return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
    }

    static bool skip_white_space(const u8chr **data)
    {
        while (**data != U8_NULL_CHR && is_whitespace(**data))
        {
            (*data)++;
        }

        return **data != U8_NULL_CHR;
    }

    /// \desc Validates that the characters pointed to by start are a valid json formatted number.
    /// \return True if the number format is valid, else false.
    static bool is_valid_number(u8chr *start)
    {
        int periods = 0;
        int exponent = 0;

        if ( *start == '-' )
        {
            ++start;
        }

        while( *start != U8_NULL_CHR )
        {
            if (is_whitespace(*start))
            {
                u8chr *p = start;
                if ( !skip_white_space((const u8chr **)&p) )
                {
                    return false;
                }
                if ( *p == ',' || *p == ']' || *p == '}' )
                {
                    return true;
                }
                return false;
            }
            if ( *start >= '0' && *start <= '9' )
            {
                ++start;
                continue;
            }
            if ( *start == '.' )
            {
                if ( exponent > 0 )
                {
                    return false;
                }
                if ( periods > 0 )
                {
                    return false;
                }
                ++periods;
                ++start;
                continue;
            }
            if ( *start == 'E' || *start == 'e' )
            {
                if ( exponent > 0 )
                {
                    return false;
                }
                ++exponent;
                ++start;
                continue;
            }
            if ( exponent > 0 && *start == '-' )
            {
                ++start;
                continue;
            }
            if ( *start == ',' || *start == ']' || *start == '}' )
            {
                break;
            }

            return false;
        }

        return *start != U8_NULL_CHR;
    }

    /// \desc Gets a dsl value with the value of the number pointed to by data.
    /// \return DslValue containing the number value. The DslValue is set to the
    ///         correct type and data is moved to position after the last digit.
    static DslValue *get_number_from_string(u8chr **data)
    {
        auto *dslValue  = new DslValue();
        dslValue->type = STRING_VALUE;

        bool is_double = false;
        u8chr ch = **data;
        while ( ch != U8_NULL_CHR )
        {
            dslValue->sValue.push_back(ch);
            if ( ch == '.' || ch == 'E' || ch == 'e' )
            {
                is_double = true;
            }
            (*data)++;
            ch = **data;
            if ( ch == ',' || ch == ']' || ch == '}' || is_whitespace(ch) )
            {
                break;
            }
        }
        if ( is_double )
        {
            dslValue->Convert(DOUBLE_VALUE);
        }
        else
        {
            dslValue->Convert(INTEGER_VALUE);
        }

        return dslValue;
    }

    /// \desc reads a string from the json text.
    static bool extract_string(const u8chr **data, U8String *str)
    {
        str->Clear();

        while (**data != U8_NULL_CHR)
        {
            // Save the char, so we can change it if need be
            wchar_t next_char = **data;

            // Escaping something?
            if (next_char == '\\')
            {
                // Move over the escape char
                (*data)++;

                // Deal with the escaped char
                switch (**data)
                {
                    case '"': next_char = '"'; break;
                    case '\\': next_char = '\\'; break;
                    case '/': next_char = '/'; break;
                    case 'b': next_char = '\b'; break;
                    case 'f': next_char = '\f'; break;
                    case 'n': next_char = '\n'; break;
                    case 'r': next_char = '\r'; break;
                    case 't': next_char = '\t'; break;
                    case 'u':
                    {
                        // We need 5 chars (4 hex + the 'u') or it's not valid
                        if (!check_at_least(*data, 5))
                        {
                            return false;
                        }

                        // Deal with the chars
                        next_char = 0;
                        for (int i = 0; i < 4; i++)
                        {
                            // Do it first to move off the 'u' and leave us on the
                            // final hex digit as we move on by one later on
                            (*data)++;

                            next_char <<= 4;

                            // Parse the hex digit
                            if (**data >= '0' && **data <= '9')
                            {
                                next_char |= (**data - '0');
                            }
                            else if (**data >= 'A' && **data <= 'F')
                            {
                                next_char |= (10 + (**data - 'A'));
                            }
                            else if (**data >= 'a' && **data <= 'f')
                            {
                                next_char |= (10 + (**data - 'a'));
                            }
                            else
                            {
                                // Invalid hex digit = invalid JSON
                                return false;
                            }
                        }
                        break;
                    }
                    default:
                        // By the spec, only the above cases are allowed
                        return false;
                }
            }
            //Have we reached the end of the string?
            else if (next_char == '"')
            {
                (*data)++;
                return true;
            }
            // Disallowed char?
            else if (next_char < ' ' && next_char != '\t')
            {
                //SPEC Violation: Allow tabs due to real world cases
                return false;
            }

            //Add the next char
            str->push_back(next_char);

            (*data)++;
        }

        // If we're here, the string ended incorrectly
        return false;
    }

    /// \desc Converts the json text from text into a dsl value collection.
    /// \param data pointer to the buffer containing the u8chr text.
    /// \return dsl value collection if successful, or nullptr if an error occurs.
    DslValue *Parse(const u8chr **data)
    {
        skip_white_space(data);
        //Is it a string?
        if (**data == '"')
        {
            U8String str = U8String();
            if (!extract_string(&(++(*data)), &str))
            {
                jsonParseErrors.push_back(new U8String("String is missing a double quote.\n"));
                return nullptr;
            }
            auto *dslValue = new DslValue(new U8String(&str));
            dslValue->type = STRING_VALUE;
            return dslValue;
        }
        //Is it a boolean?
        else if (check_at_least(*data, 4) && json_compare(*data, "true", 4))
        {
            (*data) += 4;
            auto *dslValue = new DslValue();
            dslValue->bValue = true;
            dslValue->type = BOOL_VALUE;
            return dslValue;
        }
        else if (check_at_least(*data, 5) && json_compare(*data, "false", 5))
        {
            (*data) += 5;
            auto *dslValue = new DslValue();
            dslValue->bValue = false;
            dslValue->type = BOOL_VALUE;
            return dslValue;
        }
        //Is it a null?
        else if (check_at_least(*data, 4) && json_compare(*data, "null", 4))
        {
            (*data) += 4;
            auto *dslValue = new DslValue();
            dslValue->sValue.Append("null");
            dslValue->type = STRING_VALUE;
            return dslValue;
        }
        // Is it a number?
        else if (**data == '-' || (**data >= '0' && **data <= '9'))
        {
            if (!is_valid_number((u8chr *) *data))
            {
                jsonParseErrors.push_back(new U8String("Invalid number format.\n"));
                return nullptr;
            }

            return get_number_from_string((u8chr **) data);
        }

        //An object?
        else if (**data == '{')
        {
            bool is_empty = true;

            (*data)++;

            while (**data != U8_NULL_CHR)
            {
                skip_white_space(data);

                //Special case - empty object
                if (is_empty && **data == '}')
                {
                    (*data)++;
                    U8String n = U8String();

                    n.CopyFrom(rootName);
                    n.push_back('.');
                    n.Append(rootIndex++);
                    auto *vs = new DslValue();
                    vs->type = OPEN_BLOCK;
                    vs->jsonKey.CopyFrom(&n);
                    queueValues.Enqueue(vs);

                    auto *ve = new DslValue();
                    ve->type = CLOSE_BLOCK;
                    queueValues.Enqueue(ve);
                    return ve;
                }
                is_empty = false;
                //We want a string now...
                U8String name = U8String();
                if (!extract_string(&(++(*data)), &name))
                {
                    jsonParseErrors.push_back(new U8String("Invalid string format.\n"));
                    return nullptr;
                }
                //More whitespace?
                if (!skip_white_space(data))
                {
                    jsonParseErrors.push_back(new U8String("Expected a colon after the string.\n"));
                    return nullptr;
                }
                //Need a : now
                if (*((*data)++) != ':')
                {
                    jsonParseErrors.push_back(new U8String("Key value is missing the colon after the string.\n"));
                    return nullptr;
                }
                //More whitespace?
                if (!skip_white_space(data))
                {
                    jsonParseErrors.push_back(new U8String("Missing value for the defined key.\n"));
                    return nullptr;
                }

                //If next is an object then the object needs added to the queue
                if ( **data == '{' )
                {
                    auto *vx = new DslValue();
                    vx->type = OPEN_BLOCK;
                    vx->jsonKey.CopyFrom(&name);
                    queueValues.Enqueue(vx);
                }

                //The value is here
                DslValue *value = Parse(data);
                if (value == nullptr)
                {
                    return nullptr;
                }
                value->jsonKey.CopyFrom(&name);
                queueValues.Enqueue(value);
                // More whitespace?
                if (!skip_white_space(data))
                {
                    jsonParseErrors.push_back(new U8String("Json file is incomplete.\n"));
                    return nullptr;
                }

                // End of object?
                if (**data == '}')
                {
                    (*data)++;
                    auto *vx = new DslValue();
                    vx->type = CLOSE_BLOCK;
                    return vx;
                }

                // Want a comma now
                if (**data != ',')
                {
                    jsonParseErrors.push_back(new U8String("Missing separator comma.\n"));
                    return nullptr;
                }

                (*data)++;
            }
            // Only here if we ran out of data
            jsonParseErrors.push_back(new U8String("Object specification is incomplete.\n"));
            return nullptr;
        }
        // An array?
        else if (**data == '[')
        {
            DslValue dslValue = DslValue();
            dslValue.type = INVALID_TOKEN;

            (*data)++;

            while (**data != 0)
            {
                // Whitespace at the start?
                if (!skip_white_space(data))
                {
                    jsonParseErrors.push_back(new U8String("Json file is incomplete.\n"));
                    return nullptr;
                }

                // Special case - empty array
                if (dslValue.type == INVALID_TOKEN && **data == ']')
                {
                    (*data)++;

                    U8String n = U8String();

                    n.Append(rootIndex++);
                    auto *vs = new DslValue();
                    vs->type = OPEN_BRACE;
                    vs->jsonKey.CopyFrom(&n);
                    queueValues.Enqueue(vs);

                    auto *ve = new DslValue();
                    ve->type = CLOSE_BRACE;
                    queueValues.Enqueue(ve);
                    return ve;
                }

                //If next is an array
                if ( **data == '[' )
                {
                    U8String n = U8String();

                    n.Append(rootIndex++);
                    auto *vs = new DslValue();
                    vs->type = OPEN_BRACE;
                    vs->jsonKey.CopyFrom(&n);
                    queueValues.Enqueue(vs);
                }

                // Get the value
                DslValue *value = Parse(data);
                if (value == nullptr)
                {
                    return nullptr;
                }

                // Add the value
                U8String n = U8String();

                n.Append(rootIndex++);
                auto *vs = new DslValue(value);
                vs->jsonKey.CopyFrom(&n);
                queueValues.Enqueue(vs);

                //More whitespace?
                if (!skip_white_space(data))
                {
                    jsonParseErrors.push_back(new U8String("Json file is incomplete.\n"));
                    return nullptr;
                }

                //End of array?
                if (**data == ']')
                {
                    (*data)++;

                    auto *ve = new DslValue();
                    ve->type = CLOSE_BRACE;
                    queueValues.Enqueue(ve);
                    return ve;
                }

                // Want a comma now
                if (**data != ',')
                {
                    jsonParseErrors.push_back(new U8String("Missing separator comma.\n"));
                    return nullptr;
                }

                (*data)++;
            }
            // Only here if we ran out of data
            return nullptr;
        }
        //Ran out of possibilities, it's bad!
        else
        {
            jsonParseErrors.push_back(new U8String("Json file is invalid.\n"));
            return nullptr;
        }
    }
};


#endif //DSL_CPP_JSON_PARSER_H
