//===-- JITLoader.cpp -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/lldb-private.h"
#include "lldb/Target/JITLoader.h"
#include "lldb/Target/JITLoaderList.h"

using namespace lldb;
using namespace lldb_private;

JITLoaderList::JITLoaderList()
    : m_jit_loaders_vec(), m_jit_loaders_mutex(Mutex::eMutexTypeRecursive)
{
}

JITLoaderList::~JITLoaderList()
{
}

void
JITLoaderList::Append (const JITLoaderSP &jit_loader_sp)
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    m_jit_loaders_vec.push_back(jit_loader_sp);
}

void
JITLoaderList::Remove (const JITLoaderSP &jit_loader_sp)
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    m_jit_loaders_vec.erase(std::remove(m_jit_loaders_vec.begin(),
                                        m_jit_loaders_vec.end(), jit_loader_sp),
                            m_jit_loaders_vec.end());
}

size_t
JITLoaderList::GetSize() const
{
    return m_jit_loaders_vec.size();
}

JITLoaderSP
JITLoaderList::GetLoaderAtIndex (size_t idx)
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    return m_jit_loaders_vec[idx];
}

void
JITLoaderList::DidLaunch()
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    for (auto const &jit_loader : m_jit_loaders_vec)
        jit_loader->DidLaunch();
}

void
JITLoaderList::DidAttach()
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    for (auto const &jit_loader : m_jit_loaders_vec)
        jit_loader->DidAttach();
}

void
JITLoaderList::ModulesDidLoad(ModuleList &module_list)
{
    Mutex::Locker locker(m_jit_loaders_mutex);
    for (auto const &jit_loader : m_jit_loaders_vec)
        jit_loader->ModulesDidLoad(module_list);
}
