//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "item_attributes.h"
#include "filehandle.h"

ItemAttributes::ItemAttributes() : 
	attributes(NULL)
{
}

ItemAttributes::ItemAttributes(const ItemAttributes& o)
{
	if(o.attributes)
		attributes = newd ItemAttributeMap(*o.attributes);
}

ItemAttributes::~ItemAttributes()
{
	delete attributes;
}

void ItemAttributes::createAttributes()
{
	if(!attributes)
		attributes = newd ItemAttributeMap;
}

ItemAttributeMap ItemAttributes::getAttributes() const
{
	if (attributes)
		return *attributes;
	return ItemAttributeMap();
}

void ItemAttributes::setAttribute(const std::string& key, const std::string& value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, int32_t value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, float value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, bool value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::eraseAttribute(const std::string& key)
{
	if(!attributes)
		return;
	
	ItemAttributeMap::iterator iter = attributes->find(key);

	if(iter != attributes->end())
		attributes->erase(iter);
}

const std::string* ItemAttributes::getStringAttribute(const std::string& key) const
{
	if(!attributes)
		return NULL;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getString();
	return NULL;
}

const int32_t* ItemAttributes::getIntegerAttribute(const std::string& key) const
{
	if(!attributes)
		return NULL;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getInteger();
	return NULL;
}

const float* ItemAttributes::getFloatAttribute(const std::string& key) const
{
	if(!attributes)
		return NULL;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getFloat();
	return NULL;
}

const bool* ItemAttributes::getBooleanAttribute(const std::string& key) const
{
	if(!attributes)
		return NULL;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getBoolean();
	return NULL;
}

bool ItemAttributes::hasStringAttribute(const std::string& key) const
{
	return getStringAttribute(key) != NULL;
}

bool ItemAttributes::hasIntegerAttribute(const std::string& key) const
{
	return getIntegerAttribute(key) != NULL;
}

bool ItemAttributes::hasFloatAttribute(const std::string& key) const
{
	return getFloatAttribute(key) != NULL;
}

bool ItemAttributes::hasBooleanAttribute(const std::string& key) const
{
	return getBooleanAttribute(key) != NULL;
}


// Attribute type
// Can hold either int, bool or std::string
// Without using newd to allocate them

ItemAttribute::ItemAttribute() : type(ItemAttribute::NONE)
{
}


ItemAttribute::ItemAttribute(const std::string& str) : type(ItemAttribute::STRING)
{
	new(data) std::string(str);
}

ItemAttribute::ItemAttribute(int32_t i) : type(ItemAttribute::INTEGER)
{
	*reinterpret_cast<int*>(data) = i;
}

ItemAttribute::ItemAttribute(float f) : type(ItemAttribute::FLOAT)
{
	*reinterpret_cast<float*>(data) = f;
}

ItemAttribute::ItemAttribute(bool b)
{
	*reinterpret_cast<bool*>(data) = b;
}

ItemAttribute::ItemAttribute(const ItemAttribute& o) : type(ItemAttribute::NONE)
{
	*this = o;
}
	
ItemAttribute& ItemAttribute::operator=(const ItemAttribute& o)
{
	if(&o == this)
		return *this;

	clear();
	type = o.type;
	if(type == STRING)
		new(data) std::string(*reinterpret_cast<const std::string*>(&o.data));
	else if(type == INTEGER)
		*reinterpret_cast<int32_t*>(data) = *reinterpret_cast<const int32_t*>(&o.data);
	else if(type == FLOAT)
		*reinterpret_cast<float*>(data) = *reinterpret_cast<const float*>(&o.data);
	else if(type == BOOLEAN)
		*reinterpret_cast<bool*>(data) = *reinterpret_cast<const bool*>(&o.data);
	else
		type = NONE;
	
	return *this;

}

ItemAttribute::~ItemAttribute()
{
	clear();
}

void ItemAttribute::clear()
{
	if(type == STRING){
		(reinterpret_cast<std::string*>(&data))->~basic_string();
		type = NONE;
	}
}

void ItemAttribute::set(const std::string& str)
{
	clear();
	type = STRING;
	new(data) std::string(str);
}

void ItemAttribute::set(int32_t i)
{
	clear();
	type = INTEGER;
	*reinterpret_cast<int32_t*>(&data) = i;
}

void ItemAttribute::set(float y)
{
	clear();
	type = FLOAT;
	*reinterpret_cast<float*>(&data) = y;
}

void ItemAttribute::set(bool b)
{
	clear();
	type = BOOLEAN;
	*reinterpret_cast<bool*>(&data) = b;
}


const std::string* ItemAttribute::getString() const
{
	if(type == STRING)
		return reinterpret_cast<const std::string*>(&data);
	return NULL;
}

const int32_t* ItemAttribute::getInteger() const
{
	if(type == INTEGER)
		return reinterpret_cast<const int32_t*>(&data);
	return NULL;
}

const float* ItemAttribute::getFloat() const
{
	if(type == FLOAT)
		return reinterpret_cast<const float*>(&data);
	return NULL;
}

const bool* ItemAttribute::getBoolean() const
{
	if(type == BOOLEAN)
		return reinterpret_cast<const bool*>(&data);
	return NULL;
}

bool ItemAttributes::unserializeAttributeMap(const IOMap& maphandle, BinaryNode* stream)
{
	uint16_t n;
	if(stream->getU16(n)){
		createAttributes();

		std::string key;
		ItemAttribute attrib;

		while(n--){
			if(!stream->getString(key))
				return false;
			if(!attrib.unserialize(maphandle, stream))
				return false;
			(*attributes)[key] = attrib;
		}
	}
	return true;
}

void ItemAttributes::serializeAttributeMap(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	// Maximum of 65535 attributes per item
	f.addU16(std::min((size_t)0xFFFF, attributes->size()));

	ItemAttributeMap::const_iterator attribute = attributes->begin();
	int i = 0;
	while(attribute != attributes->end() && i <= 0xFFFF){
		const std::string& key = attribute->first;
		if(key.size() > 0xFFFF)
			f.addString(key.substr(0, 65535));
		else
			f.addString(key);

		attribute->second.serialize(maphandle, f);
		++attribute, ++i;
	}
}

bool ItemAttribute::unserialize(const IOMap& maphandle, BinaryNode* stream)
{
	// Read type
	uint8_t rtype;
	stream->getU8(rtype);

	// Read contents
	switch(rtype){
		case STRING:
		{
			std::string str;
			if(!stream->getLongString(str))
				return false;
			set(str);
			break;
		}
		case INTEGER:
			uint32_t u32;
			if(!stream->getU32(u32))
				return false;
			set(*reinterpret_cast<int32_t*>(&u32));
			break;
		case FLOAT:
		{
			uint32_t u32;
			if(!stream->getU32(u32))
				return false;
			set(*reinterpret_cast<float*>(&u32));
			break;
		}
		case BOOLEAN:
		{
			uint8_t b;
			if(!stream->getU8(b))
				return false;
			set(b != 0);
		}
		default:
			break;
	}
	return true;
}

void ItemAttribute::serialize(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	// Write type
	f.addU8((uint8_t)(type));

	// Write contents
	switch(type){
		case STRING:
			f.addLongString(*getString());
			break;
		case INTEGER:
			f.addU32(*(uint32_t*)getInteger());
			break;
		case FLOAT:
			f.addU32(*(uint32_t*)getFloat());
			break;
		case BOOLEAN:
			f.addU8(*(uint8_t*)getBoolean());
		default:
			break;
	}
}