//
// Random - A plugin for selecting mesh elements at random
//

#pragma once

#include <lxsdk/lxu_attributes.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxu_select.hpp>
#include <lxsdk/lxu_vector.hpp>

#include <lxsdk/lx_channelui.hpp>
#include <lxsdk/lx_draw.hpp>
#include <lxsdk/lx_handles.hpp>
#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_plugin.hpp>
#include <lxsdk/lx_pmodel.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_tool.hpp>
#include <lxsdk/lx_toolui.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_vp.hpp>

#include <lxsdk/lx_select.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_value.hpp>

#include <iostream>

namespace Random
{

#ifndef LXx_OVERRIDE
#define LXx_OVERRIDE override
#endif

    /*
     * Basic tool and tool model methods are defined here. The
     * attributes interface is inherited from the utility class.
     */

    class CRandom : public CLxImpl_Tool, public CLxImpl_ToolModel, public CLxDynamicAttributes, public CLxImpl_ChannelUI
    {
    public:
        CRandom();

        void        tool_Reset() LXx_OVERRIDE;
        LXtObjectID tool_VectorType() LXx_OVERRIDE;
        const char* tool_Order() LXx_OVERRIDE;
        LXtID4      tool_Task() LXx_OVERRIDE;
        void        tool_Evaluate(ILxUnknownID vts) LXx_OVERRIDE;

        unsigned    tmod_Flags() LXx_OVERRIDE;
        void        tmod_Initialize(ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags) LXx_OVERRIDE;
        LxResult    tmod_Enable(ILxUnknownID obj) LXx_OVERRIDE;
        const char* tmod_Haul(unsigned index) LXx_OVERRIDE;

        LxResult atrui_DisableMsg(unsigned int index, ILxUnknownID msg) LXx_OVERRIDE;
        LxResult atrui_UIHints(unsigned int index, ILxUnknownID hints) LXx_OVERRIDE;

        bool         TestVertex(unsigned int& primary_index);
        CLxUser_Mesh GetInstance(CLxUser_Mesh& base);

        CLxUser_LogService       s_log;
        CLxUser_LayerService     s_layer;
        CLxUser_VectorType       v_type;
        CLxUser_SelectionService s_sel;
        CLxUser_MeshService      mesh_svc;

        unsigned offset_view;
        unsigned offset_screen;
        unsigned offset_falloff;
        unsigned offset_subject;
        unsigned offset_input;
        unsigned offset_event;
        unsigned offset_center;
        unsigned offset_xfrm;
        unsigned mode_select;

        LXtItemType m_itemType;
    };

    void initialize();

};  // namespace Random
