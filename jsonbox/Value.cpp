#include "Value.h"

#include <cassert>
#include <stack>
#include <sstream>
#include <list>

#include "UTFConvert.h"

namespace JsonBox {

	const std::string Value::EMPTY_STRING = std::string();
	const double Value::EMPTY_DOUBLE = 0.0;
	const Object Value::EMPTY_OBJECT = Object();
	const Array Value::EMPTY_ARRAY = Array();

	std::string Value::escapeCharacters(const std::string& str) {
		std::string result = str;

		for(size_t i = 0; i < result.length(); ++i) {
			if(result[i] == '"') {
				result.replace(i, 2, "\\\"");
				++i;
			} else if(result[i] == '\\') {
				result.replace(i, 2, "\\\\");
				++i;
			} else if(result[i] == '/') {
				result.replace(i, 2, "\\/");
				++i;
			} else if(result[i] == '\b') {
				result.replace(i, 2, "\\b");
				++i;
			} else if(result[i] == '\f') {
				result.replace(i, 2, "\\f");
				++i;
			} else if(result[i] == '\n') {
				result.replace(i, 2, "\\n");
				++i;
			} else if(result[i] == '\r') {
				result.replace(i, 2, "\\r");
				++i;
			} else if(result[i] == '\t') {
				result.replace(i, 2, "\\t");
				++i;
			}
		}

		return result;
	}

	Value::Value() : type(Type::NULL_VALUE) {
		valuePointer.stringValue = NULL;
	}

	Value::Value(std::istream& input) : type(Type::NULL_VALUE) {
		valuePointer.stringValue = NULL;
		loadFromStream(input);
	}

	Value::Value(const std::string& newString) : type(Type::NULL_VALUE) {
		valuePointer.stringValue = NULL;
		setString(newString);
	}

	Value::Value(const char* newString) : type(Type::NULL_VALUE) {
		valuePointer.stringValue = NULL;
		setString(newString);
	}

	Value::Value(int newInt) : type(Type::NULL_VALUE) {
		valuePointer.intValue = NULL;
		setInt(newInt);
	}

	Value::Value(double newDouble) : type(Type::NULL_VALUE) {
		valuePointer.doubleValue = NULL;
		setDouble(newDouble);
	}

	Value::Value(const Object& newObject) : type(Type::NULL_VALUE) {
		valuePointer.objectValue = NULL;
		setObject(newObject);
	}

	Value::Value(const Array& newArray) : type(Type::NULL_VALUE) {
		valuePointer.arrayValue = NULL;
		setArray(newArray);
	}

	Value::Value(bool newBoolean) : type(Type::NULL_VALUE) {
		valuePointer.boolValue = NULL;
		setBoolean(newBoolean);
	}

	Value::Value(const Value& src) : type(Type::NULL_VALUE) {
		valuePointer.stringValue = NULL;
		setValue(src.valuePointer, src.type);
	}

	Value::~Value() {
		switch(type) {
		case Type::STRING:

			if(valuePointer.stringValue) {
				delete valuePointer.stringValue;
			}

			break;
		case Type::INTEGER:

			if(valuePointer.intValue) {
				delete valuePointer.intValue;
			}

			break;
		case Type::DOUBLE:

			if(valuePointer.doubleValue) {
				delete valuePointer.doubleValue;
			}

			break;
		case Type::OBJECT:

			if(valuePointer.objectValue) {
				delete valuePointer.objectValue;
			}

			break;
		case Type::ARRAY:

			if(valuePointer.arrayValue) {
				delete valuePointer.arrayValue;
			}

			break;
		case Type::BOOLEAN:

			if(valuePointer.boolValue) {
				delete valuePointer.boolValue;
			}

			break;
		default:
			break;
		}
	}

	Value& Value::operator=(const Value& src) {
		if(this != &src) {
			setValue(src.valuePointer, src.type);
		}

		return *this;
	}

	Type::Enum Value::getType() const {
		return type;
	}

	bool Value::isString() const {
		return type == Type::STRING;
	}

	bool Value::isInteger() const {
		return type == Type::INTEGER;
	}

	bool Value::isDouble() const {
		return type == Type::DOUBLE;
	}

	bool Value::isObject() const {
		return type == Type::OBJECT;
	}

	bool Value::isArray() const {
		return type == Type::ARRAY;
	}

	bool Value::isBoolean() const {
		return type == Type::BOOLEAN;
	}

	bool Value::isNull() const {
		return type == Type::NULL_VALUE;
	}

	const std::string& Value::getString() const {
		if(type == Type::STRING) {
			assert(valuePointer.stringValue);
			return *valuePointer.stringValue;
		} else {
			return EMPTY_STRING;
		}
	}

	void Value::setString(std::string const& newString) {
		setValue(ValueDataPointer(&newString), Type::STRING);
	}

	int Value::getInt() const {
		if(type == Type::INTEGER) {
			assert(valuePointer.intValue);
			return *valuePointer.intValue;
		} else {
			return EMPTY_INT;
		}
	}

	void Value::setInt(int newInt) {
		setValue(ValueDataPointer(&newInt), Type::INTEGER);
	}

	double Value::getDouble() const {
		if(type == Type::DOUBLE) {
			assert(valuePointer.doubleValue);
			return *valuePointer.doubleValue;
		} else {
			return EMPTY_DOUBLE;
		}
	}

	void Value::setDouble(double newDouble) {
		setValue(ValueDataPointer(&newDouble), Type::DOUBLE);
	}

	const Object& Value::getObject() const {
		if(type == Type::OBJECT) {
			assert(valuePointer.objectValue);
			return *valuePointer.objectValue;
		} else {
			return EMPTY_OBJECT;
		}
	}

	void Value::setObject(const Object& newObject) {
		setValue(ValueDataPointer(&newObject), Type::OBJECT);
	}

	const Array& Value::getArray() const {
		if(type == Type::ARRAY) {
			assert(valuePointer.arrayValue);
			return *valuePointer.arrayValue;
		} else {
			return EMPTY_ARRAY;
		}
	}

	void Value::setArray(const Array& newArray) {
		setValue(ValueDataPointer(&newArray), Type::ARRAY);
	}

	bool Value::getBoolean() const {
		if(type == Type::BOOLEAN) {
			assert(valuePointer.boolValue);
			return *valuePointer.boolValue;
		} else {
			return EMPTY_BOOL;
		}
	}

	void Value::setBoolean(bool newBoolean) {
		setValue(ValueDataPointer(&newBoolean), Type::BOOLEAN);
	}

	void Value::setNull() {
		ValueDataPointer tmp;
		tmp.stringValue = NULL;
		setValue(tmp, Type::NULL_VALUE);
	}

	void Value::loadFromStream(std::istream& input) {
		char currentCharacter;

		// We check that the stream is in UTF-8.
		char encoding[2];
		input.get(encoding[0]);
		input.get(encoding[1]);

		if(encoding[0] != '\0' && encoding[1] != '\0') {
			// We put the characters back.
			input.putback(encoding[1]);
			input.putback(encoding[0]);

			bool noErrors = true;

			while(noErrors && !input.eof()) {
				input.get(currentCharacter);

				if(currentCharacter == '"') {
					setString("");
					readString(input, *valuePointer.stringValue);
					noErrors = false;
					std::cout << "string read: " << getString() << std::endl;
				} else if(currentCharacter == '{') {
					setObject(Object());
					readObject(input, *valuePointer.objectValue);
					noErrors = false;
					std::cout << "object read: " << getObject() << std::endl;
				} else if(currentCharacter == '[') {
					setArray(Array());
					readArray(input, *valuePointer.arrayValue);
					noErrors = false;
					std::cout << "array read: " << getArray() << std::endl;
				} else if(currentCharacter == 'n') {
					// We try to read the literal 'null'.
					if(!input.eof()) {
						input.get(currentCharacter);

						if(currentCharacter == 'u') {
							if(!input.eof()) {
								input.get(currentCharacter);

								if(currentCharacter == 'l') {
									if(!input.eof()) {
										input.get(currentCharacter);

										if(currentCharacter == 'l') {
											setNull();
											noErrors = false;
											std::cout << "null read: " << *this << std::endl;
										} else {
											std::cout << "invalid characters found" << std::endl;
										}
									} else {
										std::cout << "json input ends incorrectly" << std::endl;
									}
								} else {
									std::cout << "invalid characters found" << std::endl;
								}
							} else {
								std::cout << "json input ends incorrectly" << std::endl;
							}
						} else {
							std::cout << "invalid characters found" << std::endl;
						}
					} else {
						std::cout << "json input ends incorrectly" << std::endl;
					}
				} else if(currentCharacter == '-' || (currentCharacter >= '0' && currentCharacter <= '9')) {
					// Numbers can't start with zeroes.
					input.putback(currentCharacter);
					readNumber(input, *this);
					noErrors = false;
					std::cout << "number read: " << *this << std::endl;
				} else if(currentCharacter == 't') {
					// We try to read the boolean literal 'true'.
					if(!input.eof()) {
						input.get(currentCharacter);
						if(currentCharacter == 'r') {
							if(!input.eof()) {
								input.get(currentCharacter);
								if(currentCharacter == 'u') {
									if(!input.eof()) {
										input.get(currentCharacter);
										if(currentCharacter == 'e') {
											setBoolean(true);
											noErrors = false;
											std::cout << "Boolean read: " << *this << std::endl;
										}
									}
								}
							}
						}
					}
				} else if(currentCharacter == 'f') {
					// We try to read the boolean literal 'false'.
					if(!input.eof()) {
						input.get(currentCharacter);
						if(currentCharacter == 'a') {
							if(!input.eof()) {
								input.get(currentCharacter);
								if(currentCharacter == 'l') {
									if(!input.eof()) {
										input.get(currentCharacter);
										if(currentCharacter == 's') {
											if(!input.eof()) {
												input.get(currentCharacter);
												if(currentCharacter == 'e') {
													setBoolean(false);
													noErrors = false;
													std::cout << "Boolean read: " << *this << std::endl;
												}
											}
										}
									}
								}
							}
						}
					}
				} else if(!isWhiteSpace(currentCharacter)) {
					std::cout << "Invalid character found: '" << currentCharacter << "'" << std::endl;
				}
			}
		} else {
			std::cout << "File is not in UTF-8, not parsing." << std::endl;
		}
	}

	void Value::loadFromFile(const std::string& filePath) {
		std::ifstream file;
		file.open(filePath.c_str());

		if(file.is_open()) {
			loadFromStream(file);
			file.close();
		} else {
			std::cout << "Failed to open file to load the json: " << filePath << std::endl;
		}
	}

	void Value::writeToStream(std::ostream& output) const {
		output << *this;
	}

	void Value::writeToFile(const std::string& filePath) const {
		std::ofstream file;
		file.open(filePath.c_str());

		if(file.is_open()) {
			writeToStream(file);
			file.close();
		} else {
			std::cout << "Failed to open file to write the json into: " << filePath << std::endl;
		}
	}

	Value::ValueDataPointer::ValueDataPointer(): stringValue(NULL) {
	}

	Value::ValueDataPointer::ValueDataPointer(const std::string* newConstStringValue) :
		constStringValue(newConstStringValue) {
	}

	Value::ValueDataPointer::ValueDataPointer(const int* newConstIntValue) :
		constIntValue(newConstIntValue) {
	}

	Value::ValueDataPointer::ValueDataPointer(const double* newConstDoubleValue) :
		constDoubleValue(newConstDoubleValue) {
	}

	Value::ValueDataPointer::ValueDataPointer(const Object* newConstObjectValue) :
		constObjectValue(newConstObjectValue) {
	}

	Value::ValueDataPointer::ValueDataPointer(const Array* newConstArrayValue) :
		constArrayValue(newConstArrayValue) {
	}

	Value::ValueDataPointer::ValueDataPointer(const bool* newConstBoolValue) :
		constBoolValue(newConstBoolValue) {
	}

	bool Value::isHexDigit(char digit) {
		return (digit >= '0' && digit <= '9') || (digit >= 'a' && digit <= 'f') ||
		       (digit >= 'A' && digit <= 'F');
	}

	bool Value::isWhiteSpace(char whiteSpace) {
		return whiteSpace == ' ' || whiteSpace == '\t' || whiteSpace == '\n' ||
		       whiteSpace == '\r';
	}

	void Value::readString(std::istream& input, std::string& result) {
		bool noErrors = true, noUnicodeError = true;
		char currentCharacter, tmpCharacter;
		std::stringstream constructing;
		std::string tmpStr(4, ' ');
		std::stringstream tmpSs;
		RB_Char32 tmpInt;
		RB_String32 tmpStr32;
		unsigned int tmpCounter;

		// As long as there aren't any errors and that we haven't reached the
		// end of the input stream.
		while(noErrors && !input.eof()) {
			input.get(currentCharacter);

			if(currentCharacter & 0x80) {
				constructing << currentCharacter;
			} else if(currentCharacter == '\\') {
				if(!input.eof()) {
					input.get(tmpCharacter);

					switch(tmpCharacter) {
					case '"':
						constructing << '"';
						break;
					case '\\':
						constructing << '\\';
						break;
					case '/':
						constructing << '/';
						break;
					case 'b':
						constructing << '\b';
						break;
					case 'f':
						constructing << '\f';
						break;
					case 'n':
						constructing << '\n';
						break;
					case 'r':
						constructing << '\r';
						break;
					case 't':
						constructing << '\t';
						break;
					case 'u':
						// TODO: Check for utf16 surrogate pairs.
						tmpCounter = 0;
						tmpStr.clear();
						tmpStr = "    ";
						noUnicodeError = true;

						while(tmpCounter < 4 && !input.eof()) {
							input.get(tmpCharacter);

							if(isHexDigit(tmpCharacter)) {
								tmpStr[tmpCounter] = tmpCharacter;
							} else {
								noUnicodeError = false;
								std::cout << "Invalid \\u character, skipping it." << std::endl;
							}

							++tmpCounter;
						}

						if(noUnicodeError) {
							tmpSs.str("");
							tmpSs << std::hex << tmpStr;
							tmpSs >> tmpInt;
							tmpStr32.clear();
							tmpStr32.push_back(tmpInt);
							tmpStr = RedBox::UTFConvert::encodeToUTF8(tmpStr32);
							constructing << tmpStr;
						}

						break;
					default:
						break;
					}
				}
			} else if(currentCharacter == '"') {
				result = constructing.str();
				//std::cout << constructing.str() << std::endl;
				noErrors = false;
			} else {
				constructing << currentCharacter;
			}
		}
	}

	void Value::readObject(std::istream& input, Object& result) {
		bool noErrors = true;
		char currentCharacter;
		std::string tmpString;

		while(noErrors && !input.eof()) {
			input.get(currentCharacter);

			if(currentCharacter == '"') {
				// We read the object's member's name.
				readString(input, tmpString);
				currentCharacter = input.peek();
				// We read white spaces until the next non white space.
				readToNonWhiteSpace(input, currentCharacter);

				if(!input.eof()) {

					// We make sure it's the right character.
					if(currentCharacter == ':') {
						// We read until the value starts.
						readToNonWhiteSpace(input, currentCharacter);

						if(!input.eof()) {
							// We put the character back and we load the value
							// from the stream.
							input.putback(currentCharacter);
							result[tmpString].loadFromStream(input);

							while(!input.eof() && currentCharacter != ',' &&
							        currentCharacter != '}') {
								input.get(currentCharacter);
							}

							if(currentCharacter == '}') {
								// We are done reading the object.
								noErrors = false;
							}
						}
					}
				}
			} else if(!isWhiteSpace(currentCharacter)) {
				std::cout << "Expected '\"', got '" << currentCharacter << "', ignoring it." << std::endl;
			}
		}
	}

	void Value::readArray(std::istream& input, Array& result) {
		bool notDone = true;
		char currentChar;
		std::list<Value> constructing;
		
		while(notDone && !input.eof()) {
			input.get(currentChar);
			
			if(!isWhiteSpace(currentChar)) {
				input.putback(currentChar);
				constructing.push_back(Value());
				constructing.back().loadFromStream(input);
				
				while(!input.eof() && currentChar != ',' &&
					  currentChar != ']') {
					input.get(currentChar);
				}
				
				if(currentChar == ']') {
					notDone = false;
				}
			}
		}
		result.resize(constructing.size());
		Array::iterator i2 = result.begin();
		for(std::list<Value>::iterator i = constructing.begin();
			i != constructing.end() && i2 != result.end(); ++i) {
			
			*i2 = *i;
			
			++i2;
		}
	}

	void Value::readNumber(std::istream& input, JsonBox::Value& result) {
		bool notDone = true, inFraction = false, inExponent = false;
		char currentCharacter;
		std::stringstream constructing;
		if(!input.eof() && input.peek() == '0') {
			// We make sure there isn't more than one zero.
			input.get(currentCharacter);
			
			if(input.peek() == '0') {
				notDone = false;
			} else {
				input.putback(currentCharacter);
			}
		}
		while(notDone && !input.eof()) {
			input.get(currentCharacter);
			if(currentCharacter == '-') {
				if(constructing.str().empty()) {
					constructing << currentCharacter;
				} else {
					std::cout << "Expected a digit, '.', 'e' or 'E', got '" << currentCharacter << "' instead, ignoring it." << std::endl;
				}
			} else if(currentCharacter >= '0' && currentCharacter <= '9') {
				constructing << currentCharacter;
			} else if(currentCharacter == '.') {
				if(!inFraction && !inExponent) {
					inFraction = true;
					constructing << currentCharacter;
				}
			} else if(currentCharacter == 'e' || currentCharacter == 'E') {
				if(!inExponent) {
					inExponent = true;
					constructing << currentCharacter;
					if(!input.eof() && (input.peek() == '-' || input.peek() == '+')) {
						input.get(currentCharacter);
						constructing << currentCharacter;
					}
				}
			} else {
				input.putback(currentCharacter);
				notDone = false;
			}
		}
		if(inFraction || inExponent) {
			double doubleResult;
			constructing >> doubleResult;
			result.setDouble(doubleResult);
		} else {
			int intResult;
			constructing >> intResult;
			result.setInt(intResult);
		}
	}

	void Value::readToNonWhiteSpace(std::istream& input, char& currentCharacter) {
		do {
			input.get(currentCharacter);
		} while(!input.eof() && isWhiteSpace(currentCharacter));
	}

	void Value::setValue(ValueDataPointer newValuePointer,
	                     Type::Enum newType) {
		if(newType != Type::UNKNOWN) {
			if(type != Type::NULL_VALUE && type != newType) {
				// The current type isn't null and the new type is different.
				switch(type) {
				case Type::STRING:

					if(valuePointer.stringValue) {
						delete valuePointer.stringValue;
						valuePointer.stringValue = NULL;
					}

					break;
				case Type::INTEGER:

					if(valuePointer.intValue) {
						delete valuePointer.intValue;
						valuePointer.intValue = NULL;
					}

					break;
				case Type::DOUBLE:

					if(valuePointer.doubleValue) {
						delete valuePointer.doubleValue;
						valuePointer.doubleValue = NULL;
					}

					break;
				case Type::OBJECT:

					if(valuePointer.objectValue) {
						delete valuePointer.objectValue;
						valuePointer.objectValue = NULL;
					}

					break;
				case Type::ARRAY:

					if(valuePointer.arrayValue) {
						delete valuePointer.arrayValue;
						valuePointer.arrayValue = NULL;
					}

					break;
				case Type::BOOLEAN:

					if(valuePointer.boolValue) {
						delete valuePointer.boolValue;
						valuePointer.boolValue = NULL;
					}

					break;
				default:
					break;
				}

				type = newType;

				switch(type) {
				case Type::STRING:
					valuePointer.stringValue = new std::string(*newValuePointer.constStringValue);
					break;
				case Type::INTEGER:
					valuePointer.intValue = new int(*newValuePointer.constIntValue);
					break;
				case Type::DOUBLE:
					valuePointer.doubleValue = new double(*newValuePointer.constDoubleValue);
					break;
				case Type::OBJECT:
					valuePointer.objectValue = new Object(*newValuePointer.constObjectValue);
					break;
				case Type::ARRAY:
					valuePointer.arrayValue = new Array(*newValuePointer.constArrayValue);
					break;
				case Type::BOOLEAN:
					valuePointer.boolValue = new bool(*newValuePointer.constBoolValue);
					break;
				default:
					break;
				}
			} else if(type == newType) {
				// The new value is of the same type.
				switch(type) {
				case Type::STRING:
					assert(valuePointer.stringValue && newValuePointer.constStringValue);
					*valuePointer.stringValue = *newValuePointer.constStringValue;
					break;
				case Type::INTEGER:
					assert(valuePointer.intValue && newValuePointer.constIntValue);
					*valuePointer.intValue = *newValuePointer.constIntValue;
					break;
				case Type::DOUBLE:
					assert(valuePointer.doubleValue && newValuePointer.constDoubleValue);
					*valuePointer.doubleValue = *newValuePointer.doubleValue;
					break;
				case Type::OBJECT:
					assert(valuePointer.objectValue && newValuePointer.constObjectValue);
					*valuePointer.objectValue = *newValuePointer.constObjectValue;
					break;
				case Type::ARRAY:
					assert(valuePointer.arrayValue && newValuePointer.constArrayValue);
					*valuePointer.arrayValue = *newValuePointer.constArrayValue;
					break;
				case Type::BOOLEAN:
					assert(valuePointer.boolValue && newValuePointer.constBoolValue);
					*valuePointer.boolValue = *newValuePointer.constBoolValue;
					break;
				default:
					break;
				}
			} else {
				// The current type is null and the new type isn't
				type = newType;

				switch(type) {
				case Type::STRING:
					valuePointer.stringValue = new std::string(*newValuePointer.constStringValue);
					break;
				case Type::INTEGER:
					valuePointer.intValue = new int(*newValuePointer.constIntValue);
					break;
				case Type::DOUBLE:
					valuePointer.doubleValue = new double(*newValuePointer.constDoubleValue);
					break;
				case Type::OBJECT:
					valuePointer.objectValue = new Object(*newValuePointer.constObjectValue);
					break;
				case Type::ARRAY:
					valuePointer.arrayValue = new Array(*newValuePointer.constArrayValue);
					break;
				case Type::BOOLEAN:
					valuePointer.boolValue = new bool(*newValuePointer.constBoolValue);
					break;
				default:
					break;
				}
			}
		}
	}

	std::ostream& operator<<(std::ostream& output, const Value& v) {
		switch(v.type) {
		case Type::STRING:
			output << '"' << Value::escapeCharacters(v.getString()) << '"';
			break;
		case Type::INTEGER:
			output << v.getInt();
			break;
		case Type::DOUBLE:
			output << v.getDouble();
			break;
		case Type::OBJECT:
			output << v.getObject();
			break;
		case Type::ARRAY:
			output << v.getArray();
			break;
		case Type::BOOLEAN:
			output << ((v.getBoolean()) ? ("true") : ("false"));
			break;
		case Type::NULL_VALUE:
			output << "null";
			break;
		default:
			break;
		}

		return output;
	}

	std::ostream& operator<<(std::ostream& output, const Array& a) {
		output << '[';

		for(Array::const_iterator i = a.begin(); i != a.end(); ++i) {
			if(i != a.begin()) {
				output << ", ";
			}

			output << *i;
		}

		output << ']';
		return output;
	}

	std::ostream& operator<<(std::ostream& output, const Object& o) {
		output << '{';

		for(std::map<std::string, Value>::const_iterator i = o.begin();
		        i != o.end(); ++i) {
			if(i != o.begin()) {
				output << ", ";
			}

			output << '"' << Value::escapeCharacters(i->first) << "\" : " << i->second;
		}

		output << '}';
		return output;
	}
}
