//===-- SBTypeCategory.cpp ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/lldb-python.h"

#include "lldb/API/SBTypeCategory.h"

#include "lldb/API/SBTypeFilter.h"
#include "lldb/API/SBTypeFormat.h"
#include "lldb/API/SBTypeSummary.h"
#include "lldb/API/SBTypeSynthetic.h"
#include "lldb/API/SBTypeNameSpecifier.h"
#include "lldb/API/SBStream.h"

#include "lldb/Core/Debugger.h"
#include "lldb/DataFormatters/DataVisualization.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/ScriptInterpreter.h"

using namespace lldb;
using namespace lldb_private;

typedef std::pair<lldb::TypeCategoryImplSP,user_id_t> ImplType;

SBTypeCategory::SBTypeCategory() :
m_opaque_sp()
{
}

SBTypeCategory::SBTypeCategory (const char* name) :
m_opaque_sp()
{
    DataVisualization::Categories::GetCategory(ConstString(name), m_opaque_sp);
}

SBTypeCategory::SBTypeCategory (const lldb::SBTypeCategory &rhs) :
m_opaque_sp(rhs.m_opaque_sp)
{
}

SBTypeCategory::~SBTypeCategory ()
{
}

bool
SBTypeCategory::IsValid() const
{
    return (m_opaque_sp.get() != NULL);
}

bool
SBTypeCategory::GetEnabled ()
{
    if (!IsValid())
        return false;
    return m_opaque_sp->IsEnabled();
}

void
SBTypeCategory::SetEnabled (bool enabled)
{
    if (!IsValid())
        return;
    if (enabled)
        DataVisualization::Categories::Enable(m_opaque_sp);
    else
        DataVisualization::Categories::Disable(m_opaque_sp);
}

const char*
SBTypeCategory::GetName()
{
    if (!IsValid())
        return NULL;
    return m_opaque_sp->GetName();
}

uint32_t
SBTypeCategory::GetNumFormats ()
{
    if (!IsDefaultCategory())
        return 0;
    
    return DataVisualization::ValueFormats::GetCount();
}

uint32_t
SBTypeCategory::GetNumSummaries ()
{
    if (!IsValid())
        return 0;
    return m_opaque_sp->GetSummaryNavigator()->GetCount() + m_opaque_sp->GetRegexSummaryNavigator()->GetCount();
}

uint32_t
SBTypeCategory::GetNumFilters ()
{
    if (!IsValid())
        return 0;
    return m_opaque_sp->GetFilterNavigator()->GetCount() + m_opaque_sp->GetRegexFilterNavigator()->GetCount();
}

#ifndef LLDB_DISABLE_PYTHON
uint32_t
SBTypeCategory::GetNumSynthetics ()
{
    if (!IsValid())
        return 0;
    return m_opaque_sp->GetSyntheticNavigator()->GetCount() + m_opaque_sp->GetRegexSyntheticNavigator()->GetCount();
}
#endif

lldb::SBTypeNameSpecifier
SBTypeCategory::GetTypeNameSpecifierForFilterAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeNameSpecifier();
    return SBTypeNameSpecifier(m_opaque_sp->GetTypeNameSpecifierForFilterAtIndex(index));
}

lldb::SBTypeNameSpecifier
SBTypeCategory::GetTypeNameSpecifierForFormatAtIndex (uint32_t index)
{
    if (!IsDefaultCategory())
        return SBTypeNameSpecifier();
    return SBTypeNameSpecifier(DataVisualization::ValueFormats::GetTypeNameSpecifierForFormatAtIndex(index));
}

lldb::SBTypeNameSpecifier
SBTypeCategory::GetTypeNameSpecifierForSummaryAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeNameSpecifier();
    return SBTypeNameSpecifier(m_opaque_sp->GetTypeNameSpecifierForSummaryAtIndex(index));
}

#ifndef LLDB_DISABLE_PYTHON
lldb::SBTypeNameSpecifier
SBTypeCategory::GetTypeNameSpecifierForSyntheticAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeNameSpecifier();
    return SBTypeNameSpecifier(m_opaque_sp->GetTypeNameSpecifierForSyntheticAtIndex(index));
}
#endif

SBTypeFilter
SBTypeCategory::GetFilterForType (SBTypeNameSpecifier spec)
{
    if (!IsValid())
        return SBTypeFilter();
    
    if (!spec.IsValid())
        return SBTypeFilter();
    
    lldb::SyntheticChildrenSP children_sp;
    
    if (spec.IsRegex())
        m_opaque_sp->GetRegexFilterNavigator()->GetExact(ConstString(spec.GetName()), children_sp);
    else
        m_opaque_sp->GetFilterNavigator()->GetExact(ConstString(spec.GetName()), children_sp);
        
    if (!children_sp)
        return lldb::SBTypeFilter();
    
    TypeFilterImplSP filter_sp = std::static_pointer_cast<TypeFilterImpl>(children_sp);
    
    return lldb::SBTypeFilter(filter_sp);

}
SBTypeFormat
SBTypeCategory::GetFormatForType (SBTypeNameSpecifier spec)
{
    if (!IsDefaultCategory())
        return SBTypeFormat();
        
    if (!spec.IsValid())
        return SBTypeFormat();
    
    if (spec.IsRegex())
        return SBTypeFormat();
    
    return SBTypeFormat(DataVisualization::ValueFormats::GetFormat(ConstString(spec.GetName())));
}

#ifndef LLDB_DISABLE_PYTHON
SBTypeSummary
SBTypeCategory::GetSummaryForType (SBTypeNameSpecifier spec)
{
    if (!IsValid())
        return SBTypeSummary();
    
    if (!spec.IsValid())
        return SBTypeSummary();
    
    lldb::TypeSummaryImplSP summary_sp;
    
    if (spec.IsRegex())
        m_opaque_sp->GetRegexSummaryNavigator()->GetExact(ConstString(spec.GetName()), summary_sp);
    else
        m_opaque_sp->GetSummaryNavigator()->GetExact(ConstString(spec.GetName()), summary_sp);
    
    if (!summary_sp)
        return lldb::SBTypeSummary();
    
    return lldb::SBTypeSummary(summary_sp);
}
#endif // LLDB_DISABLE_PYTHON

#ifndef LLDB_DISABLE_PYTHON
SBTypeSynthetic
SBTypeCategory::GetSyntheticForType (SBTypeNameSpecifier spec)
{
    if (!IsValid())
        return SBTypeSynthetic();
    
    if (!spec.IsValid())
        return SBTypeSynthetic();
    
    lldb::SyntheticChildrenSP children_sp;
    
    if (spec.IsRegex())
        m_opaque_sp->GetRegexSyntheticNavigator()->GetExact(ConstString(spec.GetName()), children_sp);
    else
        m_opaque_sp->GetSyntheticNavigator()->GetExact(ConstString(spec.GetName()), children_sp);
    
    if (!children_sp)
        return lldb::SBTypeSynthetic();
    
    ScriptedSyntheticChildrenSP synth_sp = std::static_pointer_cast<ScriptedSyntheticChildren>(children_sp);
    
    return lldb::SBTypeSynthetic(synth_sp);
}
#endif

#ifndef LLDB_DISABLE_PYTHON
SBTypeFilter
SBTypeCategory::GetFilterAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeFilter();
    lldb::SyntheticChildrenSP children_sp = m_opaque_sp->GetSyntheticAtIndex((index));
    
    if (!children_sp.get())
        return lldb::SBTypeFilter();
    
    TypeFilterImplSP filter_sp = std::static_pointer_cast<TypeFilterImpl>(children_sp);
    
    return lldb::SBTypeFilter(filter_sp);
}
#endif

SBTypeFormat
SBTypeCategory::GetFormatAtIndex (uint32_t index)
{
    if (!IsDefaultCategory())
        return SBTypeFormat();
    return SBTypeFormat(DataVisualization::ValueFormats::GetFormatAtIndex((index)));
}

#ifndef LLDB_DISABLE_PYTHON
SBTypeSummary
SBTypeCategory::GetSummaryAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeSummary();
    return SBTypeSummary(m_opaque_sp->GetSummaryAtIndex((index)));
}
#endif

#ifndef LLDB_DISABLE_PYTHON
SBTypeSynthetic
SBTypeCategory::GetSyntheticAtIndex (uint32_t index)
{
    if (!IsValid())
        return SBTypeSynthetic();
    lldb::SyntheticChildrenSP children_sp = m_opaque_sp->GetSyntheticAtIndex((index));
    
    if (!children_sp.get())
        return lldb::SBTypeSynthetic();
    
    ScriptedSyntheticChildrenSP synth_sp = std::static_pointer_cast<ScriptedSyntheticChildren>(children_sp);
    
    return lldb::SBTypeSynthetic(synth_sp);
}
#endif

bool
SBTypeCategory::AddTypeFormat (SBTypeNameSpecifier type_name,
                               SBTypeFormat format)
{
    if (!IsDefaultCategory())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (!format.IsValid())
        return false;
    
    if (type_name.IsRegex())
        return false;
    
    DataVisualization::ValueFormats::Add(ConstString(type_name.GetName()), format.GetSP());
    
    return true;
}

bool
SBTypeCategory::DeleteTypeFormat (SBTypeNameSpecifier type_name)
{
    if (!IsDefaultCategory())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (type_name.IsRegex())
        return false;

    return DataVisualization::ValueFormats::Delete(ConstString(type_name.GetName()));
}

#ifndef LLDB_DISABLE_PYTHON
bool
SBTypeCategory::AddTypeSummary (SBTypeNameSpecifier type_name,
                                SBTypeSummary summary)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (!summary.IsValid())
        return false;
    
    // FIXME: we need to iterate over all the Debugger objects and have each of them contain a copy of the function
    // since we currently have formatters live in a global space, while Python code lives in a specific Debugger-related environment
    // this should eventually be fixed by deciding a final location in the LLDB object space for formatters
    if (summary.IsFunctionCode())
    {
        void *name_token = (void*)ConstString(type_name.GetName()).GetCString();
        const char* script = summary.GetData();
        StringList input; input.SplitIntoLines(script, strlen(script));
        uint32_t num_debuggers = lldb_private::Debugger::GetNumDebuggers();
        bool need_set = true;
        for (uint32_t j = 0;
             j < num_debuggers;
             j++)
        {
            DebuggerSP debugger_sp = lldb_private::Debugger::GetDebuggerAtIndex(j);
            if (debugger_sp)
            {
                ScriptInterpreter* interpreter_ptr = debugger_sp->GetCommandInterpreter().GetScriptInterpreter();
                if (interpreter_ptr)
                {
                    std::string output;
                    if (interpreter_ptr->GenerateTypeScriptFunction(input, output, name_token) && !output.empty())
                    {
                        if (need_set)
                        {
                            need_set = false;
                            summary.SetFunctionName(output.c_str());
                        }
                    }
                }
            }
        }
    }
    
    if (type_name.IsRegex())
        m_opaque_sp->GetRegexSummaryNavigator()->Add(lldb::RegularExpressionSP(new RegularExpression(type_name.GetName())), summary.GetSP());
    else
        m_opaque_sp->GetSummaryNavigator()->Add(ConstString(type_name.GetName()), summary.GetSP());
    
    return true;
}
#endif

bool
SBTypeCategory::DeleteTypeSummary (SBTypeNameSpecifier type_name)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (type_name.IsRegex())
        return m_opaque_sp->GetRegexSummaryNavigator()->Delete(ConstString(type_name.GetName()));
    else
        return m_opaque_sp->GetSummaryNavigator()->Delete(ConstString(type_name.GetName()));
}

bool
SBTypeCategory::AddTypeFilter (SBTypeNameSpecifier type_name,
                               SBTypeFilter filter)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (!filter.IsValid())
        return false;
    
    if (type_name.IsRegex())
        m_opaque_sp->GetRegexFilterNavigator()->Add(lldb::RegularExpressionSP(new RegularExpression(type_name.GetName())), filter.GetSP());
    else
        m_opaque_sp->GetFilterNavigator()->Add(ConstString(type_name.GetName()), filter.GetSP());
    
    return true;
}

bool
SBTypeCategory::DeleteTypeFilter (SBTypeNameSpecifier type_name)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (type_name.IsRegex())
        return m_opaque_sp->GetRegexFilterNavigator()->Delete(ConstString(type_name.GetName()));
    else
        return m_opaque_sp->GetFilterNavigator()->Delete(ConstString(type_name.GetName()));
}

#ifndef LLDB_DISABLE_PYTHON
bool
SBTypeCategory::AddTypeSynthetic (SBTypeNameSpecifier type_name,
                                  SBTypeSynthetic synth)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (!synth.IsValid())
        return false;

    // FIXME: we need to iterate over all the Debugger objects and have each of them contain a copy of the function
    // since we currently have formatters live in a global space, while Python code lives in a specific Debugger-related environment
    // this should eventually be fixed by deciding a final location in the LLDB object space for formatters
    if (synth.IsClassCode())
    {
        void *name_token = (void*)ConstString(type_name.GetName()).GetCString();
        const char* script = synth.GetData();
        StringList input; input.SplitIntoLines(script, strlen(script));
        uint32_t num_debuggers = lldb_private::Debugger::GetNumDebuggers();
        bool need_set = true;
        for (uint32_t j = 0;
             j < num_debuggers;
             j++)
        {
            DebuggerSP debugger_sp = lldb_private::Debugger::GetDebuggerAtIndex(j);
            if (debugger_sp)
            {
                ScriptInterpreter* interpreter_ptr = debugger_sp->GetCommandInterpreter().GetScriptInterpreter();
                if (interpreter_ptr)
                {
                    std::string output;
                    if (interpreter_ptr->GenerateTypeSynthClass(input, output, name_token) && !output.empty())
                    {
                        if (need_set)
                        {
                            need_set = false;
                            synth.SetClassName(output.c_str());
                        }
                    }
                }
            }
        }
    }
    
    if (type_name.IsRegex())
        m_opaque_sp->GetRegexSyntheticNavigator()->Add(lldb::RegularExpressionSP(new RegularExpression(type_name.GetName())), synth.GetSP());
    else
        m_opaque_sp->GetSyntheticNavigator()->Add(ConstString(type_name.GetName()), synth.GetSP());
    
    return true;
}

bool
SBTypeCategory::DeleteTypeSynthetic (SBTypeNameSpecifier type_name)
{
    if (!IsValid())
        return false;
    
    if (!type_name.IsValid())
        return false;
    
    if (type_name.IsRegex())
        return m_opaque_sp->GetRegexSyntheticNavigator()->Delete(ConstString(type_name.GetName()));
    else
        return m_opaque_sp->GetSyntheticNavigator()->Delete(ConstString(type_name.GetName()));
}
#endif // LLDB_DISABLE_PYTHON

bool
SBTypeCategory::GetDescription (lldb::SBStream &description, 
                lldb::DescriptionLevel description_level)
{
    if (!IsValid())
        return false;
    description.Printf("Category name: %s\n",GetName());
    return true;
}

lldb::SBTypeCategory &
SBTypeCategory::operator = (const lldb::SBTypeCategory &rhs)
{
    if (this != &rhs)
    {
        m_opaque_sp = rhs.m_opaque_sp;
    }
    return *this;
}

bool
SBTypeCategory::operator == (lldb::SBTypeCategory &rhs)
{
    if (IsValid() == false)
        return !rhs.IsValid();
    
    return m_opaque_sp.get() == rhs.m_opaque_sp.get();
    
}

bool
SBTypeCategory::operator != (lldb::SBTypeCategory &rhs)
{
    if (IsValid() == false)
        return rhs.IsValid();
    
    return m_opaque_sp.get() != rhs.m_opaque_sp.get();
}

lldb::TypeCategoryImplSP
SBTypeCategory::GetSP ()
{
    if (!IsValid())
        return lldb::TypeCategoryImplSP();
    return m_opaque_sp;
}

void
SBTypeCategory::SetSP (const lldb::TypeCategoryImplSP &typecategory_impl_sp)
{
    m_opaque_sp = typecategory_impl_sp;
}

SBTypeCategory::SBTypeCategory (const lldb::TypeCategoryImplSP &typecategory_impl_sp) :
m_opaque_sp(typecategory_impl_sp)
{
}

bool
SBTypeCategory::IsDefaultCategory()
{
    if (!IsValid())
        return false;
    
    return (strcmp(m_opaque_sp->GetName(),"default") == 0);
}

