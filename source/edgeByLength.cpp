//
// SelectEdgesByLength - A plugin for selecting edges by edge length
//

#include "edgeByLength.hpp"
#include "util.hpp"

namespace EdgeByLength
{

    static const char* SRVNAME_TOOL = "select.edgesByLength";

#define ATTRs_LENGTH   "length"
#define ATTRs_OPERATOR "operator"
#define ATTRs_DESELECT "deselect"

#define ATTRa_LENGTH   0
#define ATTRa_OPERATOR 1
#define ATTRa_DESELECT 2

    /*
     * On create we add our one tool attribute. We also allocate a vector type
     * and select mode mask.
     */
    CSelectEdgesByLength::CSelectEdgesByLength()
    {
        CLxUser_PacketService sPkt;
        CLxUser_MeshService   sMesh;

        static const LXtTextValueHint comparison_operators[] = {
            { OPERATOR_LESSTHAN, "less_than" }, { OPERATOR_EQUAL, "equal" }, { OPERATOR_GREATERTHAN, "greater_than" }, { 0, "=comparison_operators" }, 0
        };

        dyna_Add(ATTRs_LENGTH, LXsTYPE_DISTANCE);
        dyna_Add(ATTRs_OPERATOR, LXsTYPE_INTEGER);
        dyna_SetHint(ATTRa_OPERATOR, comparison_operators);
        dyna_Add(ATTRs_DESELECT, LXsTYPE_BOOLEAN);

        tool_Reset();

        sPkt.NewVectorType(LXsCATEGORY_TOOL, v_type);
        sPkt.AddPacket(v_type, LXsP_TOOL_VIEW_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_SCREEN_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_FALLOFF, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_SUBJECT2, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_INPUT_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_EVENTTRANS, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_ACTCENTER, LXfVT_GET);

        offset_view    = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_VIEW_EVENT);
        offset_screen  = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SCREEN_EVENT);
        offset_falloff = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_FALLOFF);
        offset_subject = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SUBJECT2);
        offset_input   = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_INPUT_EVENT);
        offset_event   = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_EVENTTRANS);
        offset_center  = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_ACTCENTER);
        offset_xfrm    = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_XFRM);
        mode_select    = sMesh.SetMode("select");
    }

    /*
     * Reset sets the attributes back to defaults.
     */
    void CSelectEdgesByLength::tool_Reset()
    {
        dyna_Value(ATTRa_LENGTH).SetFlt(0.0);
        dyna_Value(ATTRa_OPERATOR).SetInt(OPERATOR_EQUAL);
        dyna_Value(ATTRa_DESELECT).SetInt(0);
    }

    /*
     * Boilerplate methods that identify this as an action (state altering) tool.
     */
    LXtObjectID CSelectEdgesByLength::tool_VectorType()
    {
        return v_type.m_loc;  // peek method; does not add-ref
    }

    const char* CSelectEdgesByLength::tool_Order()
    {
        return LXs_ORD_ACTR;
    }

    LXtID4 CSelectEdgesByLength::tool_Task()
    {
        return LXi_TASK_ACTR;
    }

    /*
     * We employ the simplest possible tool model -- default hauling. We indicate
     * that we want to haul one attribute, we name the attribute, and we implement
     * Initialize() which is what to do when the tool activates or re-activates.
     * In this case set the axis to the current value.
     */
    unsigned CSelectEdgesByLength::tmod_Flags()
    {
        return LXfTMOD_I0_ATTRHAUL;
    }

    LxResult CSelectEdgesByLength::tmod_Enable(ILxUnknownID obj)
    {
        CLxUser_Message msg(obj);
        unsigned int    primary_index = 0;

        if (TestVertex(primary_index) == false)
        {
            msg.SetCode(LXe_CMD_DISABLED);
            msg.SetMessage(SRVNAME_TOOL, "NoVertex", 0);
            return LXe_DISABLED;
        }
        return LXe_OK;
    }

    const char* CSelectEdgesByLength::tmod_Haul(unsigned index)
    {
        if (index == 0)
            return ATTRs_LENGTH;
        else
            return nullptr;
    }

    void CSelectEdgesByLength::tmod_Initialize(ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags)
    {
        CLxUser_AdjustTool at(adjust);
        int                count = s_sel.Count(LXiSEL_EDGE);
        if (count == 0)
        {
            at.SetFlt(ATTRa_LENGTH, 0.0);
            return;
        }
        void*                         pkt = s_sel.ByIndex(LXiSEL_EDGE, static_cast<unsigned>(count - 1));
        CLxUser_EdgePacketTranslation edge_pkt_trans;
        edge_pkt_trans.autoInit();
        LXtPointID vrt0, vrt1;
        edge_pkt_trans.Vertices(pkt, &vrt0, &vrt1);
        CLxUser_Mesh mesh;
        edge_pkt_trans.GetMesh(pkt, mesh);
        CLxUser_Mesh  inst = GetInstance(mesh);
        CLxUser_Point point;
        LXtFVector    pos0, pos1;
        point.fromMesh(inst);
        point.Select(vrt0);
        point.Pos(pos0);
        point.Select(vrt1);
        point.Pos(pos1);
        double length = LXx_VDIST(pos0, pos1);
        at.SetFlt(ATTRa_LENGTH, length);
    }

    LxResult CSelectEdgesByLength::atrui_DisableMsg(unsigned int index, ILxUnknownID msg)
    {
        return LXe_OK;
    }

    LxResult CSelectEdgesByLength::atrui_UIHints(unsigned int index, ILxUnknownID hints)
    {
        CLxLoc_UIHints uiHints(hints);

        if (index == ATTRa_LENGTH)
        {
            uiHints.MinFloat(0.0);
        }
        return LXe_OK;
    }

    bool CSelectEdgesByLength::TestVertex(unsigned int& primary_index)
    {
        /*
         * Start the scan in read-only mode.
         */
        CLxUser_LayerScan scan;
        CLxUser_Mesh      mesh;
        unsigned          i, n, count;
        bool              ok = false;

        primary_index = 0;

        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);

        if (scan)
        {
            n = scan.NumLayers();
            for (i = 0; i < n; i++)
            {
                scan.BaseMeshByIndex(i, mesh);
                mesh.PointCount(&count);
                if (count > 0)
                {
                    ok            = true;
                    primary_index = i;
                    break;
                }
            }
            scan.Apply();
        }

        /*
         * Return false if there is no polygons in any active layers.
         */
        return ok;
    }

    CLxUser_Mesh CSelectEdgesByLength::GetInstance(CLxUser_Mesh& base)
    {
        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);
        auto n = scan.NumLayers();
        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            scan.MeshBase(i, mesh);
            if (mesh_svc.MeshToMeshID(mesh) == mesh_svc.MeshToMeshID(base))
            {
                scan.MeshInstance(i, mesh);
                return mesh;
            }
        }
        return base;
    }

    class EdgeVisitor : public CLxImpl_AbstractVisitor
    {
    public:
        bool Test(double length)
        {
            if (m_operator == CSelectEdgesByLength::OPERATOR_EQUAL)
            {
                if (MathUtil::Compare(length, m_length))
                    return true;
            }
            else if (m_operator == CSelectEdgesByLength::OPERATOR_LESSTHAN)
            {
                if (length < m_length)
                    return true;
            }
            else if (m_operator == CSelectEdgesByLength::OPERATOR_GREATERTHAN)
            {
                if (length > m_length)
                    return true;
            }
            return false;
        }

        LxResult Evaluate()
        {
            LXtPointID vrt0, vrt1;
            m_edge.Endpoints(&vrt0, &vrt1);

            LXtFVector pos0, pos1;
            m_vert.Select(vrt0);
            m_vert.Pos(pos0);
            m_vert.Select(vrt1);
            m_vert.Pos(pos1);

            double length = LXx_VDIST(pos0, pos1);

            if (Test(length) == true)
            {
                m_edges.push_back(m_edge.ID());
                return LXe_OK;
            }

            return LXe_OK;
        }

        CLxUser_Mesh           m_mesh;
        CLxUser_Edge           m_edge;
        CLxUser_Point          m_vert;
        LXtMarkMode            m_mark_pick;
        double                 m_length;
        int                    m_operator;
        std::vector<LXtEdgeID> m_edges;
    };

    /*
     * Tool evaluation uses layer scan interface to walk through all the active
     * meshes and visit all the selected polygons.
     */
    void CSelectEdgesByLength::tool_Evaluate(ILxUnknownID vts)
    {
        std::cout << "CSelectEdgesByLength::tool_Evaluate: " << std::endl;

        CLxUser_VectorStack    vec(vts);
        CLxUser_Subject2Packet subject;
        if (vec.ReadObject(offset_subject, subject) == false)
            return;

        LXpToolViewEvent* viewEvent = (LXpToolViewEvent*) vec.Read(offset_view);
        if (!viewEvent || viewEvent->type != LXi_VIEWTYPE_3D)
            return;

        EdgeVisitor vis;
        dyna_Value(ATTRa_LENGTH).GetFlt(&vis.m_length);
        dyna_Value(ATTRa_OPERATOR).GetInt(&vis.m_operator);

        if (vis.m_length < 0.0)
        {
            vis.m_length = 0.0;
            dyna_Value(ATTRa_LENGTH).SetFlt(vis.m_length);
        }

        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKEDGES | LXf_LAYERSCAN_MARKVERTS, scan);
        auto n = scan.NumLayers();

        CLxUser_EdgePacketTranslation edge_pkt_trans;
        edge_pkt_trans.autoInit();

        int deselect;
        dyna_Value(ATTRa_DESELECT).GetInt(&deselect);

        s_sel.StartBatch();

        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            scan.MeshInstance(i, mesh);
            vis.m_mesh = mesh;
            vis.m_edge.fromMesh(mesh);
            vis.m_vert.fromMesh(mesh);
            vis.m_mark_pick = mesh_svc.SetMode(LXsMARK_SELECT);

            vis.m_edge.Enum(&vis, LXiMARK_ANY);

            for (auto j = 0u; j < vis.m_edges.size(); j++)
            {
                LXtPointID vert0, vert1;
                vis.m_edge.Select(vis.m_edges[j]);
                vis.m_edge.Endpoints(&vert0, &vert1);
                void* pkt = edge_pkt_trans.Packet(vert0, vert1, nullptr, mesh);
                if (pkt)
                {
                    if (deselect)
                        s_sel.Deselect(LXiSEL_EDGE, pkt);
                    else
                        s_sel.Select(LXiSEL_EDGE, pkt);
                }
            }
        }
        s_sel.EndBatch();

        if (subject.Type() != LXiSEL_EDGE)
        {
            CLxUser_CommandService cmdSvc;
            cmdSvc.ExecuteArgString(-1, LXiCTAG_NULL, "select.type edge");
        }
    }

    /*
     * Export tool server.
     */
    void initialize()
    {
        CLxGenericPolymorph* srv;

        srv = new CLxPolymorph<CSelectEdgesByLength>;
        srv->AddInterface(new CLxIfc_Tool<CSelectEdgesByLength>);
        srv->AddInterface(new CLxIfc_ToolModel<CSelectEdgesByLength>);
        srv->AddInterface(new CLxIfc_Attributes<CSelectEdgesByLength>);
        srv->AddInterface(new CLxIfc_AttributesUI<CSelectEdgesByLength>);
        srv->AddInterface(new CLxIfc_ChannelUI<CSelectEdgesByLength>);
        lx::AddServer(SRVNAME_TOOL, srv);
    }
};  // namespace EdgeByLength
