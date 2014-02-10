///
/// file: Node.cpp
///

#include "Node.h"
#include "rapidjson/document.h"
#include <iostream>
#include <cstdio>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace conduit
{

///============================================
/// walk_schema helper
///============================================

/* use these funcs to avoid having to include rapidjson headers  in Node.h
 (rapidjson::Values resolve to a complex templated type that we can't forward declare) 
*/
void walk_schema(Node &node,
                 void *data, 
                 const rapidjson::Value &jvalue, 
                 index_t curr_offset);

void walk_schema(Node &node, 
                 const rapidjson::Value &jvalue);

///============================================
/// Node::m_empty
///============================================
Node Node::m_empty(DataType::Objects::empty());

///============================================
/// Node
///============================================
void
Node::init_defaults()
{
    m_data = NULL;
    m_alloced = false;
    m_dtype = DataType(DataType::EMPTY_T);

    m_mmaped    = false;
    m_mmap_fd   = -1;
    m_mmap_size = 0;
}

///============================================
Node::Node()
{
    init_defaults();
}

///============================================
Node::Node(const Node &node)
{
    init_defaults();
    set(node);
}

///============================================
Node::Node(const Schema &schema)

{
    init_defaults();
    walk_schema(schema.to_json());
}

///============================================
Node::Node(const Schema &schema, const std::string &stream_path, bool mmap)
{    
    init_defaults();
    if(mmap)
        conduit::Node::mmap(schema,stream_path);
    else
        load(schema,stream_path);
}


///============================================
Node::Node(const Schema &schema, std::ifstream &ifs)
{
    init_defaults();
    walk_schema(schema.to_json(),ifs);
}


///============================================
Node::Node(const Schema &schema, void *data)
{
    init_defaults();
    walk_schema(schema.to_json(),data);
}


///============================================
Node::Node(const DataType &dtype, void *data)
{    
    init_defaults();
    set(dtype,data);
}

///============================================
/* int vec types */
///============================================

///============================================
Node::Node(const std::vector<int8>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<int16>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<int32>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<int64>  &data)
{
   init_defaults();
   set(data);
}

///============================================
/* uint vec types */
///============================================

///============================================
Node::Node(const std::vector<uint8>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<uint16>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<uint32>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<uint64>  &data)
{
   init_defaults();
   set(data);
}

///============================================
/* float vec types */
///============================================

///============================================
Node::Node(const std::vector<float32>  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const std::vector<float64>  &data)
{
   init_defaults();
   set(data);
}

///============================================
/* int array types */
///============================================

///============================================
Node::Node(const int8_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const int16_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const int32_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const int64_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
/* uint array types */
///============================================

///============================================
Node::Node(const uint8_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const uint16_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const uint32_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const uint64_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
/* float arr types */
///============================================

///============================================
Node::Node(const float32_array  &data)
{
   init_defaults();
   set(data);
}

///============================================
Node::Node(const float64_array  &data)
{
   init_defaults();
   set(data);
}


///============================================
Node::Node(const DataType &dtype)
{
    init_defaults();
    set(dtype);
}

///============================================
/// int types
///============================================

///============================================
Node::Node(int8  data)
{
    init_defaults();
    set(data);
}

///============================================
Node::Node(int16  data)
{
    init_defaults();
    set(data);
}
    
///============================================
Node::Node(int32  data)
{
    init_defaults();
    set(data);
}

///============================================
Node::Node(int64  data)
{    
    init_defaults();
    set(data);
}


///============================================
/// uint types
///============================================

///============================================
Node::Node(uint8  data)
{
    init_defaults();
    set(data);
}

///============================================
Node::Node(uint16  data)
{
    init_defaults();
    set(data);
}
    
///============================================
Node::Node(uint32  data)
{
    init_defaults();
    set(data);
}

///============================================
Node::Node(uint64  data)
{
    init_defaults();
    set(data);
}

///============================================
/// float types
///============================================

///============================================
Node::Node(float32 data)
{
    init_defaults();
    set(data);
}


///============================================
Node::Node(float64 data)
{
    init_defaults();
    set(data);
}


///============================================
Node::~Node()
{
  cleanup();
}

///============================================
void
Node::reset()
{
  cleanup();
}

///============================================
void 
Node::load(const Schema &schema, const std::string &stream_path)
{
    index_t dsize = schema.total_bytes();

    alloc(dsize);
    std::ifstream ifs;
    ifs.open(stream_path.c_str());
    ifs.read((char *)m_data,dsize);
    ifs.close();       
    
    ///
    /// See Below
    ///
    m_alloced = false;
    
    walk_schema(schema.to_json(),m_data);

    ///
    /// TODO: Design Issue
    ///
    /// The bookkeeping here is not very intuitive 
    /// The walk process may reset the node, which would free
    /// our data before we can set it up. So for now, we wait  
    /// to indicate ownership until after the node is fully setup
    m_alloced = true;
}

///============================================
void 
Node::mmap(const Schema &schema, const std::string &stream_path)
{
    cleanup();
    index_t dsize = schema.total_bytes();
    Node::mmap(stream_path,dsize);

    ///
    /// See Below
    ///
    m_mmaped = false;
    
    walk_schema(schema.to_json(),m_data);

    ///
    /// TODO: Design Issue
    ///
    /// The bookkeeping here is not very intuitive 
    /// The walk process may reset the node, which would free
    /// our data before we can set it up. So for now, we wait  
    /// to indicate ownership until after the node is fully setup
    m_mmaped = true;
}



///============================================
void 
Node::set(const Node &node)
{
    if (!node.is_empty())
    {
        if (node.m_alloced) 
        {
            // TODO: compaction?
            init(node.m_dtype);
            memcpy(m_data, node.m_data, m_dtype.total_bytes());
        }
        else 
        {
            m_alloced = false;
            m_data    = node.m_data;
            m_dtype.reset(node.m_dtype);
        }

        m_entries = node.m_entries;
        m_list_data = node.m_list_data;
    }
}

///============================================
void 
Node::set(const DataType &dtype)
{
    // TODO: Is this right?
    // We need to cleanup and set the dtype w/o storage
    m_dtype.reset(dtype);
}

///============================================
/// int types
///============================================

///============================================
void 
Node::set(int8 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::int8());
    *(int8*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(int16 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::int16());
    *(int16*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(int32 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::int32());
    *(int32*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(int64 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::int64());
    *(int64*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
/// uint types
///============================================

///============================================
void 
Node::set(uint8 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::uint8());
    *(uint8*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(uint16 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::uint16());
    *(uint16*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(uint32 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::uint32());
    *(uint32*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(uint64 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::uint64());
    *(uint64*)((char*)m_data + m_dtype.element_index(0)) = data;
}

///============================================
/// float types
///============================================

///============================================
void 
Node::set(float32 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::float32());
    *(float32*)((char*)m_data + m_dtype.element_index(0)) = data;
}


///============================================
void 
Node::set(float64 data)
{
    // TODO check for compatible, don't always re-init
    // NOTE: comp check happens in init
    init(DataType::Scalars::float64());
    *(float64*)((char*)m_data + m_dtype.element_index(0)) = data;
}

///============================================
/// int vec types
///============================================

///============================================
void 
Node::set(const std::vector<int8>  &data)
{
    DataType vec_t(DataType::INT8_T,
                   (index_t)data.size(),
                   0,
                   sizeof(int8),
                   sizeof(int8),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(int8)*data.size());
}

///============================================
void 
Node::set(const std::vector<int16>  &data)
{
    DataType vec_t(DataType::INT16_T,
                   (index_t)data.size(),
                   0,
                   sizeof(int16),
                   sizeof(int16),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(int16)*data.size());
}

///============================================
void 
Node::set(const std::vector<int32>  &data)
{
    DataType vec_t(DataType::INT32_T,
                   (index_t)data.size(),
                   0,
                   sizeof(int32),
                   sizeof(int32),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(int32)*data.size());
}

///============================================
void 
Node::set(const std::vector<int64>  &data)
{
    DataType vec_t(DataType::INT64_T,
                   (index_t)data.size(),
                   0,
                   sizeof(int64),
                   sizeof(int64),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(int64)*data.size());
}


///============================================
/// uint vec types
///============================================

///============================================
void 
Node::set(const std::vector<uint8>  &data)
{
    DataType vec_t(DataType::UINT8_T,
                   (index_t)data.size(),
                   0,
                   sizeof(uint8),
                   sizeof(uint8),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(uint8)*data.size());
}

///============================================
void 
Node::set(const std::vector<uint16>  &data)
{
    DataType vec_t(DataType::UINT16_T,
                   (index_t)data.size(),
                   0,
                   sizeof(uint16),
                   sizeof(uint16),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(uint16)*data.size());
}

///============================================
void 
Node::set(const std::vector<uint32>  &data)
{
    DataType vec_t(DataType::UINT32_T,
                   (index_t)data.size(),
                   0,
                   sizeof(uint32),
                   sizeof(uint32),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(uint32)*data.size());
}

///============================================
void 
Node::set(const std::vector<uint64>  &data)
{
    DataType vec_t(DataType::UINT64_T,
                   (index_t)data.size(),
                   0,
                   sizeof(uint64),
                   sizeof(uint64),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(uint64)*data.size());
}

///============================================
/// float vec types
///============================================

///============================================
void 
Node::set(const std::vector<float32>  &data)
{
    DataType vec_t(DataType::FLOAT32_T,
                   (index_t)data.size(),
                   0,
                   sizeof(float32),
                   sizeof(float32),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(float32)*data.size());
}

///============================================
void 
Node::set(const std::vector<float64>  &data)
{
    DataType vec_t(DataType::FLOAT64_T,
                   (index_t)data.size(),
                   0,
                   sizeof(float64),
                   sizeof(float64),
                   Endianness::DEFAULT_T);
    init(vec_t);
    memcpy(m_data,&data[0],sizeof(float64)*data.size());
}

///============================================
/// int array types
///============================================

///============================================
void 
Node::set(const int8_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const int16_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const int32_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const int64_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}


///============================================
/// uint array types
///============================================

///============================================

void 
Node::set(const uint8_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const uint16_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const uint32_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const uint64_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}
///============================================
/// float array types
///============================================

///============================================
void 
Node::set(const float32_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}

///============================================
void 
Node::set(const float64_array  &data)
{
    cleanup();
    m_dtype = data.dtype();
    m_data  = data.data_ptr();
}


///============================================
void
Node::set(const Schema & schema,void* data)
{
    walk_schema(schema,data);    
}


///============================================
void
Node::set(const DataType &dtype, void *data)
{
    cleanup();
    m_alloced = false;
    m_data    = data;
    m_dtype.reset(dtype);
}

///============================================
Node &
Node::operator=(const Node &node)
{
    if(this != &node)
    {
        set(node);
    }
    return *this;
}

///============================================
Node &
Node::operator=(DataType dtype)
{
    set(dtype);
    return *this;
}

///============================================
/// uint types
///============================================

///============================================
Node &
Node::operator=(uint8 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(uint16 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(uint32 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(uint64 data)
{
    set(data);
    return *this;
}

///============================================
/// int types
///============================================

///============================================
Node &
Node::operator=(int8 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(int16 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(int32 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(int64 data)
{
    set(data);
    return *this;
}

///============================================
/// float types
///============================================

///============================================
Node &
Node::operator=(float32 data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(float64 data)
{
    set(data);
    return *this;
}

///============================================
/// int vec types
///============================================

///============================================
Node &
Node::operator=(const std::vector<int8> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<int16> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<int32> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<int64> &data)
{
    set(data);
    return *this;
}

///============================================
/// uint vec types
///============================================

///============================================
Node &
Node::operator=(const std::vector<uint8> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<uint16> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<uint32> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<uint64> &data)
{
    set(data);
    return *this;
}

///============================================
/// float vec types
///============================================

///============================================
Node &
Node::operator=(const std::vector<float32> &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const std::vector<float64> &data)
{
    set(data);
    return *this;
}

///============================================
/// int array types
///============================================

///============================================
Node &
Node::operator=(const int8_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const int16_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const int32_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const int64_array &data)
{
    set(data);
    return *this;
}

///============================================
/// uint vec types
///============================================

///============================================
Node &
Node::operator=(const uint8_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const uint16_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const uint32_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const uint64_array &data)
{
    set(data);
    return *this;
}

///============================================
/// float vec types
///============================================

///============================================
Node &
Node::operator=(const float32_array &data)
{
    set(data);
    return *this;
}

///============================================
Node &
Node::operator=(const float64_array &data)
{
    set(data);
    return *this;
}


///============================================
index_t
Node::total_bytes() const
{
    index_t res = 0;
    index_t dt_id = m_dtype.id();
    if(dt_id == DataType::OBJECT_T)
    {
        const std::map<std::string, Node> &ents = entries();
        for (std::map<std::string, Node>::const_iterator itr = ents.begin();
             itr != ents.end(); ++itr) 
        {
            res += itr->second.total_bytes();
        }
    }
    else if(dt_id == DataType::LIST_T)
    {
        const std::vector<Node> &lst = list();
        for (std::vector<Node>::const_iterator itr = lst.begin();
             itr != lst.end(); ++itr)
        {
            res += itr->total_bytes();
        }
    }
    else if (dt_id != DataType::EMPTY_T)
    {
        res = m_dtype.total_bytes();
    }
    return res;
}

///============================================
Schema
Node::schema() const
{
    return Schema(json_schema());
}


///============================================
std::string
Node::json_schema() const
{
    std::ostringstream oss;
    json_schema(oss);
    return oss.str();
}


///============================================
void
Node::json_schema(std::ostringstream &oss) const
{
    if(m_dtype.id() == DataType::OBJECT_T)
    {
        oss << "{";
        std::map<std::string,Node>::const_iterator itr;
        const std::map<std::string, Node> &ents = entries();
        bool first=true;
        for(itr = ents.begin(); itr != ents.end(); ++itr)
        {
            if(!first)
                oss << ",";
            oss << "\""<< itr->first << "\" : ";
            oss << itr->second.json_schema() << "\n";
            first=false;
        }
        oss << "}\n";
    }
    else if(m_dtype.id() == DataType::LIST_T)
    {
        oss << "[";
        std::vector<Node>::const_iterator itr;
        const std::vector<Node> &lst = list();
        bool first=true;
        for(itr = lst.begin(); itr != lst.end(); ++itr)
        {
            if(!first)
                oss << ",";
            oss << (*itr).json_schema() << "\n";
            first=false;
        }
        oss << "]\n";
    }
    else // assume data value type for now
    {
        m_dtype.json_schema(oss);
    }
}

///============================================
void
Node::serialize(std::vector<uint8> &data,bool compact) const
{
    data = std::vector<uint8>(total_bytes(),0);
    serialize(&data[0],0,compact);
}

///============================================
void
Node::serialize(const std::string &stream_path,
                bool compact) const
{
	std::ofstream ofs;
    ofs.open(stream_path.c_str());
    serialize(ofs,compact);
    ofs.close();
}


///============================================
void
Node::serialize(std::ofstream &ofs,
                bool compact) const
{
    if(m_dtype.id() == DataType::OBJECT_T)
    {
        std::map<std::string,Node>::const_iterator itr;
        const std::map<std::string,Node> &ent = entries();
        for(itr = ent.begin(); itr != ent.end(); ++itr)
        {
            itr->second.serialize(ofs);
        }
    }
    else if(m_dtype.id() == DataType::LIST_T)
    {
        std::vector<Node>::const_iterator itr;
        const std::vector<Node> &lst = list();
        for(itr = lst.begin(); itr != lst.end(); ++itr)
        {
            (*itr).serialize(ofs);
        }
    }
    else // assume data value type for now
    {
        // TODO: Compact?
        ofs.write((const char*)element_pointer(0),total_bytes());
    }
}


///============================================
void
Node::serialize(uint8 *data,index_t curr_offset,bool compact) const
{
    if(m_dtype.id() == DataType::OBJECT_T)
    {
        std::map<std::string,Node>::const_iterator itr;
        const std::map<std::string,Node> &ent = entries();
        for(itr = ent.begin(); itr != ent.end(); ++itr)
        {
            itr->second.serialize(&data[0],curr_offset);
            curr_offset+=itr->second.total_bytes();
        }
    }
    else if(m_dtype.id() == DataType::LIST_T)
    {
        std::vector<Node>::const_iterator itr;
        const std::vector<Node> &lst = list();
        for(itr = lst.begin(); itr != lst.end(); ++itr)
        {
            (*itr).serialize(&data[0],curr_offset);
            curr_offset+=(*itr).total_bytes();
        }
    }
    else // assume data value type for now
    {
        // TODO: Compact?
        memcpy(&data[curr_offset],m_data,total_bytes());
    }
}

///============================================
bool             
Node::compare(const Node &n, Node &cmp_results) const
{
/// TODO: cmp_results will describe the diffs between this & n    
}


///============================================
bool             
Node::operator==(const Node &n) const
{
/// TODO value comparison
    return false;
}

///============================================
bool             
Node::is_empty() const
{
    return  m_dtype.id() == DataType::EMPTY_T;
}


///============================================
void
Node::init_list()
{
    if (m_dtype.id() != DataType::LIST_T)
    {
        cleanup();
        m_dtype.reset(DataType::LIST_T);
    }
}

///============================================
Node&
Node::entry(const std::string &path)
{
    // fetch w/ path forces OBJECT_T
    if(m_dtype.id() != DataType::OBJECT_T)
        return empty();
        
    std::string p_curr;
    std::string p_next;
    split_path(path,p_curr,p_next);
    // find p_curr with an iterator
    std::map<std::string, Node> &ents = entries();
    std::map<std::string, Node>::iterator itr = ents.find(p_curr);
    // return Empty if the entry does not exist (static/locked case)
    if(itr == ents.end())
        return empty();
    
    if(p_next.empty())
    {
        return itr->second;
    }
    else
    {
        return itr->second.entry(p_next);
    }
}


///============================================
Node&
Node::entry(index_t idx)
{
    if(m_dtype.id() != DataType::LIST_T)
    {
        return empty();
    }
    // we could also potentially support index fetch on:
    //   OBJECT_T (imp-order)
    //   ARRAY_T -- Object array, dynamic construction of node
    return list()[idx];
}

///============================================
const Node &
Node::entry(const std::string &path) const
{
    // fetch w/ path forces OBJECT_T
    if(m_dtype.id() != DataType::OBJECT_T)
        return empty();
        
    std::string p_curr;
    std::string p_next;
    split_path(path,p_curr,p_next);
    // find p_curr with an iterator
    const std::map<std::string, Node> &ents = entries();
    std::map<std::string, Node>::const_iterator itr = ents.find(p_curr);
    // return Empty if the entry does not exist (static/locked case)
    if(itr == ents.end())
        return empty();
    
    if(p_next.empty())
    {
        return itr->second;
    }
    else
    {
        return itr->second.entry(p_next);
    }
}


///============================================
const Node &
Node::entry(index_t idx) const
{
    if(m_dtype.id() != DataType::LIST_T)
    {
        return empty();
    }
    // we could also potentially support index fetch on:
    //   OBJECT_T (imp-order)
    //   ARRAY_T -- Object array, dynamic construction of node
    return list()[idx];
}

///============================================
Node&
Node::fetch(const std::string &path)
{
    // fetch w/ path forces OBJECT_T
    if(m_dtype.id() != DataType::OBJECT_T)
        init(DataType::Objects::object());
        
    std::string p_curr;
    std::string p_next;
    split_path(path,p_curr,p_next);
    if(p_next.empty())
        return entries()[p_curr];
    else
        return entries()[p_curr].fetch(p_next);
}


///============================================
Node&
Node::fetch(index_t idx)
{
    // if(m_dtype.id() != DataType::LIST_T)
    // {
    // }
    // we could also potentially support index fetch on:
    //   OBJECT_T (imp-order)
    //   ARRAY_T -- Object array, dynamic construction of node
    return list()[idx];
}

///============================================
Node&
Node::operator[](const std::string &path)
{
    //if(!m_locked)
        return fetch(path);
    //else
    //    return entry(path);
}

///============================================
Node&
Node::operator[](index_t idx)
{
    //if(!m_locked)
        return fetch(idx);
    //else
    //    return entry(idx);
}

/// Const variants use const get
///============================================
const Node&
Node::operator[](const std::string &path) const
{
    return entry(path);
}

///============================================
const Node&
Node::operator[](index_t idx) const
{
    return entry(idx);
}



///============================================
bool           
Node::has_path(const std::string &path) const
{
    if(m_dtype.id() != DataType::OBJECT_T)
        return false;

    std::string p_curr;
    std::string p_next;
    split_path(path,p_curr,p_next);
    const std::map<std::string,Node> &ents = entries();

    if(ents.find(p_curr) == ents.end())
    {
        return false;
    }

    if(!p_next.empty())
    {
        const Node &n = ents.find(p_curr)->second;
        return n.has_path(p_next);
    }
    else
    {
        return true;
    }
}


///============================================
void
Node::paths(std::vector<std::string> &paths,bool expand) const
{
    // TODO: Imp
    // TODO: expand == True, show nested paths
}

///============================================
index_t 
Node::number_of_entries() const 
{
    // list only for now
    if(m_dtype.id() != DataType::LIST_T)
        return 0;
    return list().size();
}

///============================================
bool    
Node::remove(index_t idx)
{
    if(m_dtype.id() != DataType::LIST_T)
        return false;
    
    std::vector<Node>  &lst = list();
    if(idx > lst.size())
        return false;
    lst.erase(lst.begin() + idx);
    return true;
}

///============================================
bool    
Node::remove(const std::string &path)
{
    if(m_dtype.id() != DataType::OBJECT_T)
        return false;

    std::string p_curr;
    std::string p_next;
    split_path(path,p_curr,p_next);
    std::map<std::string,Node> &ents = entries();

    if(ents.find(p_curr) == ents.end())
    {
        return false;
    }

    if(!p_next.empty())
    {
        Node &n = ents.find(p_curr)->second;
        return n.remove(p_next);
        
    }
    else
    {
        ents.erase(p_curr);
        return true;
    }
}
///============================================
int64
Node::to_int() const
{
    switch(m_dtype.id())
    {
        case DataType::BOOL_T:  return (int64)as_bool();
        /* ints */
        case DataType::INT8_T:  return (int64)as_int8();
        case DataType::INT16_T: return (int64)as_int16();
        case DataType::INT32_T: return (int64)as_int32();
        case DataType::INT64_T: return as_int64();
        /* uints */
        case DataType::UINT8_T:  return (int64)as_uint8();
        case DataType::UINT16_T: return (int64)as_uint16();
        case DataType::UINT32_T: return (int64)as_uint32();
        case DataType::UINT64_T: return (int64)as_uint64();
        /* floats */
        case DataType::FLOAT32_T: return (int64)as_float32();
        case DataType::FLOAT64_T: return (int64)as_float64();
    }
    return 0;
    
}

///============================================
uint64
Node::to_uint() const
{
    switch(m_dtype.id())
    {
        case DataType::BOOL_T:  return (uint64)as_bool();
        /* ints */
        case DataType::INT8_T:  return (uint64)as_int8();
        case DataType::INT16_T: return (uint64)as_int16();
        case DataType::INT32_T: return (uint64)as_int32();
        case DataType::INT64_T: return (uint64)as_int64();
        /* uints */
        case DataType::UINT8_T:  return (uint64)as_uint8();
        case DataType::UINT16_T: return (uint64)as_uint16();
        case DataType::UINT32_T: return (uint64)as_uint32();
        case DataType::UINT64_T: return as_uint64();
        /* floats */
        case DataType::FLOAT32_T: return (uint64)as_float32();
        case DataType::FLOAT64_T: return (uint64)as_float64();
    }
    return 0;
}

///============================================
float64
Node::to_float() const
{
    switch(m_dtype.id())
    {
        case DataType::BOOL_T:  return (float64)as_bool();
        /* ints */
        case DataType::INT8_T:  return (float64)as_int8();
        case DataType::INT16_T: return (float64)as_int16();
        case DataType::INT32_T: return (float64)as_int32();
        case DataType::INT64_T: return (float64)as_int64();
        /* uints */
        case DataType::UINT8_T:  return (float64)as_uint8();
        case DataType::UINT16_T: return (float64)as_uint16();
        case DataType::UINT32_T: return (float64)as_uint32();
        case DataType::UINT64_T: return (float64)as_uint64();
        /* floats */
        case DataType::FLOAT32_T: return (float64)as_float32();
        case DataType::FLOAT64_T: return as_float64();
    }
    return 0.0;
}

///============================================
std::string 
Node::to_string() const
{
   std::ostringstream oss;
   to_string(oss);
   return oss.str();
}

///============================================
void
Node::to_string(std::ostringstream &oss, bool json_fmt) const
{
    switch(m_dtype.id())
    {
        case DataType::BOOL_T:
        {
            if(as_bool())
                oss << "true"; 
            else
                oss << "false"; 
            break;
        }
        /* ints */
        case DataType::INT8_T:  oss << (uint64) as_int8(); break;
        case DataType::INT16_T: oss << as_int16(); break;
        case DataType::INT32_T: oss << as_int32(); break;
        case DataType::INT64_T: oss << as_int64(); break;
        /* uints */
        case DataType::UINT8_T:  oss << (uint64) as_uint8(); break;
        case DataType::UINT16_T: oss << as_uint16(); break;
        case DataType::UINT32_T: oss << as_uint32(); break;
        case DataType::UINT64_T: oss << as_uint64(); break;
        /* floats */
        case DataType::FLOAT32_T: oss << as_float32(); break;
        case DataType::FLOAT64_T: oss << as_float64(); break;
        case DataType::BYTESTR_T: 
        {
            if(json_fmt)
                oss << "\"";
            oss << as_bytestr();
            if(json_fmt)
                oss << "\"";
            break;
        }
    }

    if(m_dtype.id() == DataType::OBJECT_T)
    {
        bool first = true;
        oss << "{";
        const std::map<std::string, Node> &ents = entries();
        for (std::map<std::string, Node>::const_iterator itr = ents.begin();
             itr != ents.end(); ++itr) 
        {
            if(!first)
                oss << ",";
            oss << " \"" << itr->first << "\" : ";
            itr->second.to_string(oss,true);
            first = false;
        }
        oss << "}\n";
    }
    else if(m_dtype.id() == DataType::LIST_T)
    {
        bool first = true;
        oss << "[" << std::endl;
        const std::vector<Node> &lst = list();
        for (std::vector<Node>::const_iterator itr = lst.begin();
             itr != lst.end(); ++itr)
        {
            if(!first)
                oss << ",";
            itr->to_string(oss,true);
            first = false;
        }
        oss << "]\n";
    }
    
}

    
///============================================
void
Node::init(const DataType& dtype)
{
    if (!m_dtype.is_compatible(dtype) || m_data == NULL)
    {
        cleanup();
        index_t dt_id = dtype.id();
        if( dt_id == DataType::OBJECT_T)
        {
            // TODO: alloc map
        }
        else if(dt_id == DataType::LIST_T)
        {
            // TODO: alloc list
        }
        else if(dt_id != DataType::EMPTY_T)
        {
            // TODO: This implies compact storage
            // TODO: should we just malloc / dealloc?
            alloc(dtype.number_of_elements()*dtype.element_bytes());
        }

        m_dtype.reset(dtype);
    }
}


///============================================
void
Node::alloc(index_t dsize)
{
    m_data = malloc(dsize);
    m_alloced = true;
    m_mmaped  = false;
}

///============================================
void
Node::mmap(const std::string &stream_path, index_t dsize)
{
    m_mmap_fd   = open(stream_path.c_str(),O_RDWR| O_CREAT);
    m_mmap_size = dsize;

    if (m_mmap_fd == -1) 
    {
	    // error
	    std::ostringstream msg;
	    msg << "<Node::mmap> failed to open: " << stream_path;
	    throw Error(msg);
    }

    m_data = ::mmap(0, dsize, PROT_READ | PROT_WRITE, MAP_SHARED, m_mmap_fd, 0);

    if (m_data == MAP_FAILED) 
    {
	    // error
        // error
	    std::ostringstream msg;
	    msg << "<Node::mmap> MAP_FAILED" << stream_path;
	    throw Error(msg);
    }
    
    m_alloced = false;
    m_mmaped  = true;
}



///============================================
void
Node::cleanup()
{
    if(m_alloced && m_data)
    {
        if(m_dtype.id() != DataType::EMPTY_T)
        {
            // scalar and array types are alloce
            // TODO: should we just malloc / dealloc?
            // using the char allocator (should we act
            free(m_data);
        }
    }   
    else if(m_mmaped && m_data)
    {
        if(munmap(m_data, m_mmap_size) == -1) 
        {
            // error
        }
        close(m_mmap_fd);
    }

    
    m_data      = NULL;
    m_alloced   = false;
    m_mmaped    = false;
    m_mmap_fd   = 0;
    m_mmap_size = 0;
    m_dtype.reset(DataType::EMPTY_T);

}
    

///============================================
std::map<std::string, Node> &  
Node::entries()
{
   return m_entries;
}

///============================================
std::vector<Node> &  
Node::list()
{
   return m_list_data;
}


///============================================
const std::map<std::string, Node> &  
Node::entries() const
{
   return m_entries;
}

///============================================
const std::vector<Node> &  
Node::list() const
{
   return m_list_data;
}


///============================================
void 
Node::walk_schema(const Schema &schema)
{
    m_data    = NULL;
    m_alloced = false;
    m_dtype.reset(DataType::OBJECT_T);
    
    rapidjson::Document document;
    document.Parse<0>(schema.to_json().c_str());
    
    conduit::walk_schema(*this,document);
}


///============================================
void 
walk_schema(Node &node, 
            const rapidjson::Value &jvalue)
{
    if (jvalue.HasMember("dtype"))
    {
        // if dtype is an object, we have a "list_of" case or tree node
        const rapidjson::Value &dt_value = jvalue["dtype"];
        if(dt_value.IsObject() && jvalue.HasMember("source"))
        {
            std::string path(jvalue["source"].GetString());
            // read source
            
            //node = Node();
            //walk_schema(node,data,jvalue,0);
        }
        else // we will alloc a data buffer that can hold all of the node data
        {
            // walk_schema(node,data,jvalue,0);
        }
    }
}


///============================================
void 
Node::walk_schema(const Schema &schema, void *data)
{
    m_data = data;
    m_alloced = false;
    m_dtype.reset(DataType::OBJECT_T);
    
    rapidjson::Document document;
    document.Parse<0>(schema.to_json().c_str());
    index_t current_offset = 0;
    conduit::walk_schema(*this,data,document,current_offset);
}

///============================================
void 
walk_schema(Node &node, 
            void *data,
            const rapidjson::Value &jvalue,
            index_t curr_offset)
{
    if(jvalue.IsObject())
    {
        /*
        static const char* kTypeNames[] = { "Null", 
                                            "False", 
                                            "True", 
                                            "Object", 
                                            "Array", 
                                            "String", 
                                            "Number" };
        */
        if (jvalue.HasMember("dtype"))
        {
            // if dtype is an object, we have a "list_of" case
            const rapidjson::Value &dt_value = jvalue["dtype"];
            if(dt_value.IsObject())
            {
                int length =1;
                if(jvalue.HasMember("length"))
                {
                    length = jvalue["length"].GetInt();
                }
                            
                // we will create `length' # of objects of obj des by dt_value
                 
                // TODO: we only need to parse this once, not leng # of times
                // but this is the easiest way to start. 
                for(int i=0;i< length;i++)
                {
                    Node curr_node(DataType::Objects::object());
                    walk_schema(curr_node,data, dt_value, curr_offset);
                    node.append(curr_node);
                    curr_offset += curr_node.total_bytes();
                }
            }
            else
            {
                // handle leaf node with explicit props
                std::string dtype_name(jvalue["dtype"].GetString());
                int length = jvalue["length"].GetInt();
                const DataType df_dtype = DataType::default_dtype(dtype_name);
                index_t type_id = df_dtype.id();
                index_t size    = df_dtype.element_bytes();
                // TODO: Parse endianness
                DataType dtype(type_id,
                               length,
                               curr_offset,
                               size, 
                               size,
                               Endianness::DEFAULT_T);
                node.set(dtype,data);
            }
        }
        else
        {
            // loop over all entries
            for (rapidjson::Value::ConstMemberIterator itr = jvalue.MemberBegin(); 
                 itr != jvalue.MemberEnd(); ++itr)
            {
                std::string entry_name(itr->name.GetString());
                Node curr_node(DataType::Objects::object());
                walk_schema(curr_node,data, itr->value, curr_offset);
                node[entry_name] = curr_node;
                curr_offset += curr_node.total_bytes();
            }
        }
    }
    else if (jvalue.IsArray()) 
    {
        for (rapidjson::SizeType i = 0; i < jvalue.Size(); i++)
        {
			Node curr_node(DataType::Objects::object());
            walk_schema(curr_node,data, jvalue[i], curr_offset);
            curr_offset += curr_node.total_bytes();
            // this will coerce to a list
            node.append(curr_node);
        }
    }
    else if(jvalue.IsString())
    {
         std::string dtype_name(jvalue.GetString());
         DataType df_dtype = DataType::default_dtype(dtype_name);
         index_t size = df_dtype.element_bytes();
         DataType dtype(df_dtype.id(),1,curr_offset,size,size,Endianness::DEFAULT_T);
         node.set(dtype,data);
    }

}


///============================================
void 
Node::split_path(const std::string &path,
                 std::string &curr,
                 std::string &next)
{
    curr.clear();
    next.clear();
    std::size_t found = path.find("/");
    if (found != std::string::npos)
    {
        curr = path.substr(0,found);
        if(found != path.size()-1)
            next = path.substr(found+1,path.size()-(found-1));
    }
    else
    {
        curr = path;
    } 
}


}
