//
// SelectEdgesByEdgeVector - A plugin for selecting edges by edge vector facing ratio
//

#include "edgeByEdgeVector.hpp"
#include "util.hpp"

namespace EdgeByEdgeVector
{

    static const char* SRVNAME_TOOL = "select.edgesByEdgeVector";

#define ATTRs_TOLERANCE "tolerance"
#define ATTRs_DESELECT  "deselect"

#define ATTRa_TOLERANCE 0
#define ATTRa_DESELECT  1

    /*
     * On create we add our one tool attribute. We also allocate a vector type
     * and select mode mask.
     */
    CSelectEdgesByEdgeVector::CSelectEdgesByEdgeVector()
    {
        CLxUser_PacketService sPkt;
        CLxUser_MeshService   sMesh;

        dyna_Add(ATTRs_TOLERANCE, LXsTYPE_ANGLE);
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
    void CSelectEdgesByEdgeVector::tool_Reset()
    {
        dyna_Value(ATTRa_TOLERANCE).SetFlt(0.0);
        dyna_Value(ATTRa_DESELECT).SetInt(0);
    }

    /*
     * Boilerplate methods that identify this as an action (state altering) tool.
     */
    LXtObjectID CSelectEdgesByEdgeVector::tool_VectorType()
    {
        return v_type.m_loc;  // peek method; does not add-ref
    }

    const char* CSelectEdgesByEdgeVector::tool_Order()
    {
        return LXs_ORD_ACTR;
    }

    LXtID4 CSelectEdgesByEdgeVector::tool_Task()
    {
        return LXi_TASK_ACTR;
    }

    /*
     * We employ the simplest possible tool model -- default hauling. We indicate
     * that we want to haul one attribute, we name the attribute, and we implement
     * Initialize() which is what to do when the tool activates or re-activates.
     * In this case set the axis to the current value.
     */
    unsigned CSelectEdgesByEdgeVector::tmod_Flags()
    {
        return LXfTMOD_I0_ATTRHAUL | LXfTMOD_I0_INPUT;
    }

    LxResult CSelectEdgesByEdgeVector::tmod_Enable(ILxUnknownID obj)
    {
        CLxUser_Message msg(obj);
        unsigned int    primary_index = 0;

        if (TestVertex(primary_index) == false)
        {
            msg.SetCode(LXe_CMD_DISABLED);
            msg.SetMessage(SRVNAME_TOOL, "NoVertex", 0);
            return LXe_DISABLED;
        }

        CLxUser_Mesh mesh;
        CLxUser_Edge edge;
        LXtMatrix4 xfrm;
        if (GetLastEdge(mesh, edge, xfrm) == false)
        {
            msg.SetCode(LXe_CMD_DISABLED);
            msg.SetMessage(SRVNAME_TOOL, "NoEdgeSelected", 0);
            return LXe_DISABLED;
        }
        return LXe_OK;
    }

    const char* CSelectEdgesByEdgeVector::tmod_Haul(unsigned index)
    {
        if (index == 0)
            return ATTRs_TOLERANCE;
        else
            return nullptr;
    }

    void CSelectEdgesByEdgeVector::tmod_Initialize(ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags)
    {
        dyna_Value(ATTRa_TOLERANCE).SetFlt(0.0);
    }

    LxResult CSelectEdgesByEdgeVector::atrui_DisableMsg(unsigned int index, ILxUnknownID msg)
    {
        return LXe_OK;
    }

    LxResult CSelectEdgesByEdgeVector::atrui_UIHints(unsigned int index, ILxUnknownID hints)
    {
        CLxLoc_UIHints uiHints(hints);

        if (index == ATTRa_TOLERANCE)
        {
            uiHints.MinFloat(0.0);
            uiHints.MaxFloat(179.0 * LXx_DEG2RAD);
        }
        return LXe_OK;
    }

    bool CSelectEdgesByEdgeVector::TestVertex(unsigned int& primary_index)
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

    CLxUser_Mesh CSelectEdgesByEdgeVector::GetInstance(CLxUser_Mesh& base, LXtMatrix4 xfrm)
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
                scan.MeshTransform(i, xfrm);
                return mesh;
            }
        }
        return base;
    }

    static CLxVector GetEdgeVector(CLxUser_Mesh& mesh, CLxUser_Edge& edge, LXtMatrix4 xfrm)
    {
        LXtPointID vrt0, vrt1;
        edge.Endpoints(&vrt0, &vrt1);

        LXtFVector    pos0, pos1;
        CLxUser_Point point;
        point.fromMesh(mesh);
        point.Select(vrt0);
        point.Pos(pos0);
        point.Select(vrt1);
        point.Pos(pos1);
        lx::Matrix4Multiply(pos0, xfrm, pos0);
        lx::Matrix4Multiply(pos1, xfrm, pos1);
        LXx_VSUB(pos1, pos0);
        CLxVector vec(pos1);
        vec.normalize();
        return vec;
    }

    bool CSelectEdgesByEdgeVector::GetLastEdge(CLxUser_Mesh& mesh, CLxUser_Edge& edge, LXtMatrix4 xfrm)
    {
        int count = s_sel.Count(LXiSEL_EDGE);
        if (count == 0)
        {
            return false;
        }
        void*                         pkt = s_sel.ByIndex(LXiSEL_EDGE, static_cast<unsigned>(count - 1));
        CLxUser_EdgePacketTranslation edge_pkt_trans;
        edge_pkt_trans.autoInit();
        LXtPointID vrt0, vrt1;
        edge_pkt_trans.Vertices(pkt, &vrt0, &vrt1);
        edge_pkt_trans.GetMesh(pkt, mesh);
        CLxUser_Mesh inst = GetInstance(mesh, xfrm);
        edge.fromMesh(inst);
        edge.SelectEndpoints(vrt0, vrt1);
        return true;
    }

    class EdgeVisitor : public CLxImpl_AbstractVisitor
    {
    public:
        bool Test(CLxVector& vector)
        {
            double angle = std::abs(MathUtil::VectorAngle(vector, m_vector));
            if ((angle <= m_tolerance) || MathUtil::Compare(angle, m_tolerance))
            {
                return true;
            }
            return false;
        }

        LxResult Evaluate()
        {
            if (m_deselect == 0)
            {
                if (m_edge.ID() == m_lastEdge.ID())
                    return LXe_OK;
            }

            CLxVector vector = GetEdgeVector(m_mesh, m_edge, m_xfrm);

            if (Test(vector) == true)
            {
                m_edges.push_back(m_edge.ID());
                return LXe_OK;
            }

            vector = vector * -1.0;
            if (Test(vector) == true)
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
        CLxVector              m_vector;
        CLxUser_Edge           m_lastEdge;
        LXtMatrix4             m_xfrm;
        int                    m_deselect;
        double                 m_tolerance;
        std::vector<LXtEdgeID> m_edges;
    };

    /*
     * Tool evaluation uses layer scan interface to walk through all the active
     * meshes and visit all the selected polygons.
     */
    void CSelectEdgesByEdgeVector::tool_Evaluate(ILxUnknownID vts)
    {
        std::cout << "** CSelectEdgesByEdgeVector::tool_Evaluate: " << std::endl;

        CLxUser_VectorStack    vec(vts);
        CLxUser_Subject2Packet subject;
        if (vec.ReadObject(offset_subject, subject) == false)
            return;

        LXpToolViewEvent* viewEvent = (LXpToolViewEvent*) vec.Read(offset_view);
        if (!viewEvent || viewEvent->type != LXi_VIEWTYPE_3D)
            return;

        EdgeVisitor vis;

        if (GetLastEdge(vis.m_mesh, vis.m_lastEdge, vis.m_xfrm) == false)
            return;

        vis.m_vector = GetEdgeVector(vis.m_mesh, vis.m_lastEdge, vis.m_xfrm);
        dyna_Value(ATTRa_TOLERANCE).GetFlt(&vis.m_tolerance);
        if (vis.m_tolerance < 0.0)
        {
            vis.m_tolerance = 0.0;
            dyna_Value(ATTRa_TOLERANCE).SetFlt(vis.m_tolerance);
        }

        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKEDGES | LXf_LAYERSCAN_MARKVERTS, scan);
        auto n = scan.NumLayers();

        CLxUser_EdgePacketTranslation edge_pkt_trans;
        edge_pkt_trans.autoInit();

        dyna_Value(ATTRa_DESELECT).GetInt(&vis.m_deselect);

        s_sel.StartBatch();

        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            vis.m_edges.clear();
            scan.MeshInstance(i, mesh);
            vis.m_mesh = mesh;
            vis.m_edge.fromMesh(mesh);
            vis.m_vert.fromMesh(mesh);
            vis.m_mark_pick = mesh_svc.SetMode(LXsMARK_SELECT);
            scan.MeshTransform(i, vis.m_xfrm);

            vis.m_edge.Enum(&vis, LXiMARK_ANY);

            for (auto j = 0u; j < vis.m_edges.size(); j++)
            {
                LXtPointID vert0, vert1;
                vis.m_edge.Select(vis.m_edges[j]);
                vis.m_edge.Endpoints(&vert0, &vert1);
                void* pkt = edge_pkt_trans.Packet(vert0, vert1, nullptr, mesh);
                if (pkt)
                {
                    if (vis.m_deselect)
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

        srv = new CLxPolymorph<CSelectEdgesByEdgeVector>;
        srv->AddInterface(new CLxIfc_Tool<CSelectEdgesByEdgeVector>);
        srv->AddInterface(new CLxIfc_ToolModel<CSelectEdgesByEdgeVector>);
        srv->AddInterface(new CLxIfc_Attributes<CSelectEdgesByEdgeVector>);
        srv->AddInterface(new CLxIfc_AttributesUI<CSelectEdgesByEdgeVector>);
        srv->AddInterface(new CLxIfc_ChannelUI<CSelectEdgesByEdgeVector>);
        lx::AddServer(SRVNAME_TOOL, srv);
    }
};  // namespace EdgeByEdgeVector
