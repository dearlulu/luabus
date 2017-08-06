/*
** repository: https://github.com/trumanzhao/luabus.git
** trumanzhao, 2017-07-09, trumanzhao@foxmail.com
*/

#include "tools.h"
#include "var_int.h"
#include "lua_socket_mgr.h"
#include "lua_socket_node.h"

EXPORT_CLASS_BEGIN(lua_socket_mgr)
EXPORT_LUA_FUNCTION(wait)
EXPORT_LUA_FUNCTION(listen)
EXPORT_LUA_FUNCTION(connect)
EXPORT_LUA_FUNCTION(set_package_size)
EXPORT_LUA_FUNCTION(set_lz_threshold)
EXPORT_LUA_FUNCTION(set_master)
EXPORT_LUA_FUNCTION(map_token)
EXPORT_CLASS_END()

lua_socket_mgr::~lua_socket_mgr()
{
}

bool lua_socket_mgr::setup(lua_State* L, int max_fd)
{
    m_lvm = L;
    m_mgr = std::make_shared<socket_mgr>();
    m_archiver = std::make_shared<lua_archiver>(1024);
    m_router = std::make_shared<socket_router>(m_mgr);
    return m_mgr->setup(max_fd);
}

int lua_socket_mgr::listen(lua_State* L)
{
    const char* ip = lua_tostring(L, 1);
    int port = (int)lua_tointeger(L, 2);
    if (ip == nullptr || port <= 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid param");
        return 2;
    }

    std::string err;
    int token = m_mgr->listen(err, ip, port);
    if (token == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }

    auto listener = new lua_socket_node(token, m_lvm, m_mgr, m_archiver, m_router);
    lua_push_object(L, listener);
    lua_pushstring(L, "ok");
    return 2;
}

int lua_socket_mgr::connect(lua_State* L)
{
    const char* ip = lua_tostring(L, 1);
    const char* port = lua_tostring(L, 2);
    if (ip == nullptr || port == nullptr)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid param");
        return 2;
    }

    std::string err;
    int token = m_mgr->connect(err, ip, port);
    if (token == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, err.c_str());
        return 2;
    }

    auto stream = new lua_socket_node(token, m_lvm, m_mgr, m_archiver, m_router);
    lua_push_object(L, stream);
    lua_pushstring(L, "ok");
    return 2;
}

void lua_socket_mgr::set_package_size(size_t size)
{
    m_archiver->set_buffer_size(size);
}

void lua_socket_mgr::set_lz_threshold(size_t size)
{
    m_archiver->set_lz_threshold(size);
}

void lua_socket_mgr::set_master(uint32_t group_idx, uint32_t token)
{
    m_router->set_master(group_idx, token);
}

int lua_socket_mgr::map_token(lua_State* L)
{
    uint32_t service_id = (uint32_t)lua_tointeger(L, 1);
    if (lua_isnil(L, 2))
    {
        m_router->erase(service_id);
    }
    else
    {
        uint32_t token = (uint32_t)lua_tointeger(L, 2);
        m_router->map_token(service_id, token);
    }
    return 0;
}

