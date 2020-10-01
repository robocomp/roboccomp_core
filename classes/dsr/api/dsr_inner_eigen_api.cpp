#include "dsr_inner_eigen_api.h"
#include "dsr_api.h"

using namespace DSR;

//InnerAPI::InnerAPI(std::shared_ptr<DSR::DSRGraph> _G)
InnerEigenAPI::InnerEigenAPI(DSR::DSRGraph *G_)
{
    G = G_;
    //update signals
    connect(G, &DSR::DSRGraph::update_edge_signal, this, &InnerEigenAPI::add_or_assign_edge_slot);
    connect(G, &DSR::DSRGraph::del_edge_signal, this, &InnerEigenAPI::del_edge_slot);
    connect(G, &DSR::DSRGraph::del_node_signal, this, &InnerEigenAPI::del_node_slot);
}

/// Computation of resultant RTMat going from A to common ancestor and from common ancestor to B (inverted)
//std::optional<InnerEigenAPI::Lists> InnerEigenAPI::setLists(const std::string &dest, const std::string &orig)
//{
//    std::list<NodeMatrix> listA, listB;
//    Mat::RTMat atotal(Mat::RTMat::Identity());
//    Mat::RTMat btotal(Mat::RTMat::Identity());
//
//  	auto an = G->get_node(orig);
//	auto bn = G->get_node(dest);
//    if ( not an.has_value() or  not bn.has_value())
//		return {};
//	auto a = an.value(); auto b = bn.value();
//
//	int minLevel = std::min(G->get_node_level(a).value_or(-1), G->get_node_level(b).value_or(-1));
//	if (minLevel == -1)
//		return {};
//	while (G->get_node_level(a).value_or(-1) >= minLevel)
//	{
//        qDebug() << "listaA" << a.id() << G->get_node_level(a).value() << G->get_parent_id(a).value();
//		auto p_node = G->get_parent_node(a);
//      	if( not p_node.has_value())
//			break;  // FIX THIS GIVING MORE INFO
//		auto edge_rt = G->get_edge_RT(p_node.value(), a.id()).value();
//		auto rtmat = G->get_edge_RT_as_rtmat(edge_rt).value();
//        atotal = rtmat * atotal;
//        listA.emplace_back(std::make_tuple(p_node.value().id(), std::move(rtmat)));   // the downwards RT link from parent to a
//        a = p_node.value();
//	}
//	while (G->get_node_level(b).value_or(-1) >= minLevel)
//	{
//        qDebug() << "listaB" << b.id() << G->get_node_level(b).value() << G->get_parent_id(b).value();
//		auto p_node = G->get_parent_node(b);
//		if(not p_node.has_value())
//			break;
//		auto edge_rt = G->get_edge_RT(p_node.value(), b.id()).value();
//		auto rtmat = G->get_edge_RT_as_rtmat(edge_rt).value();
//        btotal = rtmat.inverse() * btotal;
//        listB.emplace_front(std::make_tuple(p_node.value().id(), std::move(rtmat)));
//		b = p_node.value();
//	}
//	// from min_level up tp the common ancestor
//	while (a.id() != b.id())
//	{
//		auto p_node = G->get_node(G->get_parent_id(a).value_or(-1));
//		auto q_node = G->get_node(G->get_parent_id(b).value_or(-1));
//		if(p_node.has_value() and q_node.has_value())
//		{
//            qDebug() << "listas A&B" << p_node.value().id() << q_node.value().id();
//            auto a_edge_rt = G->get_edge_RT(p_node.value(), a.id()).value();
//            auto b_edge_rt = G->get_edge_RT(q_node.value(), b.id()).value();
//            auto a_rtmat = G->get_edge_RT_as_rtmat(a_edge_rt).value();
//            auto b_rtmat = G->get_edge_RT_as_rtmat(b_edge_rt).value();
//            atotal = a_rtmat * atotal;
//            btotal = b_rtmat.inverse() * btotal;
//            listA.emplace_back(std::make_tuple(p_node.value().id(), std::move(a_rtmat)));
//	  		listB.emplace_front(std::make_tuple(q_node.value().id(), std::move(b_rtmat)));
//			a = p_node.value();
//			b = q_node.value();
//		}
//		else
//			return {};
//	}
//    //	std::cout << "----QAUDUD--------" << std::endl;
//    //	std::cout << (atotal*btotal).matrix() << std::endl;
//    return std::make_tuple(listA, listB);
//}

////////////////////////////////////////////////////////////////////////////////////////
////// TRANSFORMATION MATRIX
////////////////////////////////////////////////////////////////////////////////////////

std::optional<Mat::RTMat> InnerEigenAPI::get_transformation_matrix(const std::string &dest, const std::string &orig)
{
   KeyTransform key = std::make_tuple(dest, orig);
    if( TransformCache::iterator it = cache.find(key) ; it != cache.end())
    {
        qInfo() << "cache hit Eigen";
        return it->second;
    }
    else
    {
        Mat::RTMat atotal(Mat::RTMat::Identity());
        Mat::RTMat btotal(Mat::RTMat::Identity());

        auto an = G->get_node(orig);
        auto bn = G->get_node(dest);
        if ( not an.has_value() or  not bn.has_value())
            return {};
        auto a = an.value(); auto b = bn.value();

        int minLevel = std::min(G->get_node_level(a).value_or(-1), G->get_node_level(b).value_or(-1));
        if (minLevel == -1)
            return {};
        while (G->get_node_level(a).value_or(-1) >= minLevel)
        {
            qDebug() << "listaA" << a.id() << G->get_node_level(a).value() << G->get_parent_id(a).value();
            auto p_node = G->get_parent_node(a);
            if( not p_node.has_value())
                break;  // FIX THIS GIVING MORE INFO
            auto edge_rt = G->get_edge_RT(p_node.value(), a.id()).value();
            auto rtmat = G->get_edge_RT_as_rtmat(edge_rt).value();
            atotal = rtmat * atotal;
            node_map[p_node.value().id()].push_back(key); // update node cache reference
            a = p_node.value();
        }
        while (G->get_node_level(b).value_or(-1) >= minLevel)
        {
            qDebug() << "listaB" << b.id() << G->get_node_level(b).value() << G->get_parent_id(b).value();
            auto p_node = G->get_parent_node(b);
            if(not p_node.has_value())
                break;
            auto edge_rt = G->get_edge_RT(p_node.value(), b.id()).value();
            auto rtmat = G->get_edge_RT_as_rtmat(edge_rt).value();
            btotal = rtmat * btotal;
            node_map[p_node.value().id()].push_back(key); // update node cache reference
            b = p_node.value();
        }
        // from min_level up tp the common ancestor
        while (a.id() != b.id())
        {
            auto p_node = G->get_node(G->get_parent_id(a).value_or(-1));
            auto q_node = G->get_node(G->get_parent_id(b).value_or(-1));
            if(p_node.has_value() and q_node.has_value())
            {
                qDebug() << "listas A&B" << p_node.value().id() << q_node.value().id();
                auto a_edge_rt = G->get_edge_RT(p_node.value(), a.id()).value();
                auto b_edge_rt = G->get_edge_RT(q_node.value(), b.id()).value();
                auto a_rtmat = G->get_edge_RT_as_rtmat(a_edge_rt).value();
                auto b_rtmat = G->get_edge_RT_as_rtmat(b_edge_rt).value();
                atotal = a_rtmat * atotal;
                btotal = b_rtmat * btotal;
                node_map[p_node.value().id()].push_back(key); // update node cache reference
                node_map[q_node.value().id()].push_back(key); // update node cache reference
                a = p_node.value();
                b = q_node.value();
            }
            else
                return {};
        }
        // update node cache reference
        int32_t dst_id = G->get_node(dest).value().id();
        node_map[dst_id].push_back(key);
        int32_t orig_id = G->get_node(orig).value().id();
        node_map[orig_id].push_back(key);

        // update cache
        auto ret = atotal*btotal.inverse();
        cache[key] = ret;
        return ret;
    }
}

//std::optional<Mat::RTMat> InnerEigenAPI::get_transformation_matrix(const std::string &dest, const std::string &orig)
//{
//	Mat::RTMat ret(Mat::RTMat::Identity());
//   KeyTransform key = std::make_tuple(dest, orig);
//    if( transform_cache::iterator it = cache.find(key) ; it != cache.end())
//        ret = it->second;
//    else
//	{
//        auto lists = setLists(dest, orig);
//        if(!lists.has_value())
//            return {};
//        auto &[listA, listB] = lists.value();
//
//        for(auto &[id, mat]: listA )
//        {
//            ret = mat * ret;
//            node_map[id].push_back(key); // update node cache reference
//        }
//        for(auto &[id, mat]: listB )
//        {
//            ret = mat.inverse() * ret;
//            node_map[id].push_back(key); // update node cache reference
//        }
//        std::cout << "----ORIG--------" << std::endl;
//        std::cout << ret.matrix() << std::endl;
//        // update node cache reference
//        int32_t dst_id = G->get_node(dest).value().id();
//        node_map[dst_id].push_back(key);
//        int32_t orig_id = G->get_node(orig).value().id();
//        node_map[orig_id].push_back(key);
//
//        // update cache
//        cache[key] = ret;
//    }
//	return ret;
//}

////////////////////////////////////////////////////////////////////////////////////////
////// TRANSFORM
////////////////////////////////////////////////////////////////////////////////////////
std::optional<Eigen::Vector3d> InnerEigenAPI::transform(const std::string &dest, const Eigen::Vector3d &vector, const std::string &orig)
{
    //std::cout <<__FUNCTION__ << " " << initVec << std::endl;
    auto tm = get_transformation_matrix(dest, orig);
    //std::cout << __FUNCTION__ << " " << tm.value().matrix().format(CleanFmt) << std::endl;
    if(tm.has_value())
    {
        //qInfo() << __FUNCTION__ << " " << tm.value().matrix().size() << initVec.homogeneous().size();
        return (tm.value() * vector.homogeneous());
    }
    else
        return {};
}

std::optional<Eigen::Vector3d> InnerEigenAPI::transform( const std::string &dest, const std::string & orig)
 {
 	return transform(dest, Eigen::Vector3d(0.,0.,0.), orig);
 }

std::optional<Mat::Vector6d> InnerEigenAPI::transform_axis(const std::string &dest, const Mat::Vector6d &vector, const std::string &orig)
{
    auto tm = get_transformation_matrix(dest, orig);
    if(tm.has_value())
    {
        const Mat::RTMat rtmat = tm.value();
        const Eigen::Vector3d a = rtmat * (vector.head(3).homogeneous());
        const Mat::Rot3D R(Eigen::AngleAxisd(vector(3), Eigen::Vector3d::UnitX()) *
                           Eigen::AngleAxisd(vector(4), Eigen::Vector3d::UnitY()) *
                           Eigen::AngleAxisd(vector(5), Eigen::Vector3d::UnitZ()));
        const Eigen::Vector3d b = ( rtmat.rotation() * R).eulerAngles(0,1,2);
        Mat::Vector6d ret;
        ret << a(0),  a(1), a(2), b(0), b(1), b(2);
        return ret;
    }
    else
        return {};
 }

std::optional<Mat::Vector6d> InnerEigenAPI::transform_axis( const std::string &dest,  const std::string & orig)
 {
    Mat::Vector6d v;
	return transform_axis(dest, v.Zero(), orig);
 }

////////////////////////////////////////////////////////////////////////
/// SLOTS ==> used to remove cached transforms when node/edge changes
///////////////////////////////////////////////////////////////////////
void InnerEigenAPI::add_or_assign_edge_slot(const std::int32_t from, const std::int32_t to, const std::string& edge_type)
{
    if(edge_type == "RT")
    {
        remove_cache_entry(from);
        remove_cache_entry(to);
    }
}
void InnerEigenAPI::del_node_slot(const std::int32_t id)
{
    remove_cache_entry(id);
}
void InnerEigenAPI::del_edge_slot(const std::int32_t from, const std::int32_t to, const std::string &edge_type)
{
    if(edge_type == "RT")
    {
        remove_cache_entry(from);
        remove_cache_entry(to);
    }
}
void InnerEigenAPI::remove_cache_entry(const std::int32_t id)
{
    NodeReference::iterator it = node_map.find(id);
    if(it != node_map.end())
    {
        for(KeyTransform key: it->second)
        {
            cache.erase(key);
        }
    }
    node_map.erase(id);
}