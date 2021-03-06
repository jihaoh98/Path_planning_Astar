#include <cmath>
#include "Astar_searcher.h"

using namespace std;
using namespace Eigen;

// 初始化地图信息,每一个节点都带上coor以及index
void AstarPathFinder::initGridMap(double _resolution, Vector3d global_xyz_l, Vector3d global_xyz_u, int max_x_id, int max_y_id, int max_z_id)
{   
    gl_xl = global_xyz_l(0);
    gl_yl = global_xyz_l(1);
    gl_zl = global_xyz_l(2);

    gl_xu = global_xyz_u(0);
    gl_yu = global_xyz_u(1);
    gl_zu = global_xyz_u(2);
    
    GLX_SIZE = max_x_id;
    GLY_SIZE = max_y_id;
    GLZ_SIZE = max_z_id;
    GLYZ_SIZE  = GLY_SIZE * GLZ_SIZE;
    GLXYZ_SIZE = GLX_SIZE * GLYZ_SIZE;

    resolution = _resolution;
    inv_resolution = 1.0 / _resolution;    

    data = new uint8_t[GLXYZ_SIZE];
    memset(data, 0, GLXYZ_SIZE * sizeof(uint8_t));
    
    GridNodeMap = new GridNodePtr ** [GLX_SIZE];
    for(int i = 0; i < GLX_SIZE; i++)
    {
        GridNodeMap[i] = new GridNodePtr * [GLY_SIZE];
        for(int j = 0; j < GLY_SIZE; j++)
        {
            GridNodeMap[i][j] = new GridNodePtr [GLZ_SIZE];
            for( int k = 0; k < GLZ_SIZE;k++)
            {
                Vector3i tmpIdx(i,j,k);
                Vector3d pos = gridIndex2coord(tmpIdx);
                GridNodeMap[i][j][k] = new GridNode(tmpIdx, pos);
            }
        }
    }
}

// 初始化每个节点的配置信息
void AstarPathFinder::resetGrid(GridNodePtr ptr)
{
    ptr->id = 0;
    ptr->cameFrom = NULL;
    ptr->gScore = inf;
    ptr->fScore = inf;
}

void AstarPathFinder::resetUsedGrids()
{   
    for(int i=0; i < GLX_SIZE ; i++)
        for(int j=0; j < GLY_SIZE ; j++)
            for(int k=0; k < GLZ_SIZE ; k++)
                resetGrid(GridNodeMap[i][j][k]);
}

void AstarPathFinder::setObs(const double coord_x, const double coord_y, const double coord_z)
{
    if( coord_x < gl_xl  || coord_y < gl_yl  || coord_z <  gl_zl || 
        coord_x >= gl_xu || coord_y >= gl_yu || coord_z >= gl_zu )
        return;

    int idx_x = static_cast<int>( (coord_x - gl_xl) * inv_resolution);
    int idx_y = static_cast<int>( (coord_y - gl_yl) * inv_resolution);
    int idx_z = static_cast<int>( (coord_z - gl_zl) * inv_resolution);      

    data[idx_x * GLYZ_SIZE + idx_y * GLZ_SIZE + idx_z] = 1;
}

vector<Vector3d> AstarPathFinder::getVisitedNodes()
{   
    vector<Vector3d> visited_nodes;
    // ROS_INFO("The total size of the map  is %d", GLXYZ_SIZE);  // 62500
    for(int i = 0; i < GLX_SIZE; i++)
        for(int j = 0; j < GLY_SIZE; j++)
            for(int k = 0; k < GLZ_SIZE; k++)
            {   
                //if(GridNodeMap[i][j][k]->id != 0) // visualize all nodes in open and close list
                if(GridNodeMap[i][j][k]->id == -1)  // visualize nodes in close list only
                    visited_nodes.push_back(GridNodeMap[i][j][k]->coord);
            }

    ROS_WARN("visited_nodes size : %d", visited_nodes.size());
    return visited_nodes;
}

Vector3d AstarPathFinder::gridIndex2coord(const Vector3i & index) 
{
    Vector3d pt;

    pt(0) = ((double)index(0) + 0.5) * resolution + gl_xl;
    pt(1) = ((double)index(1) + 0.5) * resolution + gl_yl;
    pt(2) = ((double)index(2) + 0.5) * resolution + gl_zl;

    return pt;
}

Vector3i AstarPathFinder::coord2gridIndex(const Vector3d & pt) 
{
    Vector3i idx;
    idx <<  min( max( int( (pt(0) - gl_xl) * inv_resolution), 0), GLX_SIZE - 1),
            min( max( int( (pt(1) - gl_yl) * inv_resolution), 0), GLY_SIZE - 1),
            min( max( int( (pt(2) - gl_zl) * inv_resolution), 0), GLZ_SIZE - 1);                  
  
    return idx;
}

Eigen::Vector3d AstarPathFinder::coordRounding(const Eigen::Vector3d & coord)
{
    return gridIndex2coord(coord2gridIndex(coord));
}

inline bool AstarPathFinder::isOccupied(const Eigen::Vector3i & index) const
{
    return isOccupied(index(0), index(1), index(2));
}

inline bool AstarPathFinder::isFree(const Eigen::Vector3i & index) const
{
    return isFree(index(0), index(1), index(2));
}

inline bool AstarPathFinder::isOccupied(const int & idx_x, const int & idx_y, const int & idx_z) const 
{
    return  (idx_x >= 0 && idx_x < GLX_SIZE && idx_y >= 0 && idx_y < GLY_SIZE && idx_z >= 0 && idx_z < GLZ_SIZE && 
            (data[idx_x * GLYZ_SIZE + idx_y * GLZ_SIZE + idx_z] == 1));
}

inline bool AstarPathFinder::isFree(const int & idx_x, const int & idx_y, const int & idx_z) const 
{
    return (idx_x >= 0 && idx_x < GLX_SIZE && idx_y >= 0 && idx_y < GLY_SIZE && idx_z >= 0 && idx_z < GLZ_SIZE && 
           (data[idx_x * GLYZ_SIZE + idx_y * GLZ_SIZE + idx_z] < 1));
}

inline void AstarPathFinder::AstarGetSucc(GridNodePtr currentPtr, vector<GridNodePtr> & neighborPtrSets, vector<double> & edgeCostSets)
{   
    neighborPtrSets.clear();
    edgeCostSets.clear();
    /*
    *
    STEP 4: finish AstarPathFinder::AstarGetSucc yourself 
    please write your code below
    *
    *
    */
    int current_x = currentPtr->index(0);
    int current_y = currentPtr->index(1);
    int current_z = currentPtr->index(2);

    for (int i = current_x - 1; i <= current_x + 1; i++)
    {
        for (int j = current_y - 1; j <= current_y + 1; j++)
        {
            for (int k = current_z - 1; k <= current_z + 1; k++)
            {
                if(i == current_x && j == current_y && k == current_z)
                    continue;
                if(isFree(i, j, k))
                {
                    neighborPtrSets.push_back(GridNodeMap[i][j][k]);
                    double edgecost = getdistance(currentPtr, GridNodeMap[i][j][k]);
                    edgeCostSets.push_back(edgecost);
                }    
            }
            
        }
    }

}

double AstarPathFinder::getdistance(GridNodePtr node1, GridNodePtr node2)
{
    double distance;
    distance  = (node1->coord(0) - node2->coord(0)) * (node1->coord(0) - node2->coord(0));
    distance += (node1->coord(1) - node2->coord(1)) * (node1->coord(1) - node2->coord(1));
    distance += (node1->coord(2) - node2->coord(2)) * (node1->coord(2) - node2->coord(2));
    distance = sqrt(distance);

    return distance;
}

double AstarPathFinder::getHeu(GridNodePtr node1, GridNodePtr node2, int flag)
{
    /* 
    choose possible heuristic function you want
    Manhattan, Euclidean, Diagonal, or 0 (Dijkstra)
    Remember tie_breaker learned in lecture, add it here ?
    *
    *
    *
    STEP 1: finish the AstarPathFinder::getHeu , which is the heuristic function
    please write your code below
    *
    *
    */
    // Euclidean
    double Euclidean_heuristic;
    Euclidean_heuristic  = (node1->coord(0) - node2->coord(0)) * (node1->coord(0) - node2->coord(0));
    Euclidean_heuristic += (node1->coord(1) - node2->coord(1)) * (node1->coord(1) - node2->coord(1));
    Euclidean_heuristic += (node1->coord(2) - node2->coord(2)) * (node1->coord(2) - node2->coord(2));
    Euclidean_heuristic = sqrt(Euclidean_heuristic);

    // Manhattan more fast
    double Manhattan_heuristic;
    Manhattan_heuristic = abs(node1->coord(0) - node2->coord(0));
    Manhattan_heuristic += abs(node1->coord(1) - node2->coord(1)) + abs(node1->coord(2) - node2->coord(2));
    

    // Diagonal 
    double Diagonal_heuristic;
    double temp;
    double max1 = abs(node1->coord(0) - node2->coord(0));
    double max2 = abs(node1->coord(1) - node2->coord(1));
    double max3 = abs(node1->coord(2) - node2->coord(2));

    if(max2 > max1)
    {
        temp = max1;
        max1 = max2;
        max2 = temp;
    }
    if(max3 > max1)
    {
        temp = max1;
        max1 = max3;
        max3 = temp;
    }
    if(max3 > max2)
    {
        temp = max2;
        max2 = max3;
        max3 = temp;
    }
    Diagonal_heuristic = sqrt(3) * max3 + sqrt(2) * max2 + max1 - max2;

    double Tie_heuristic = Euclidean_heuristic * (1 +  0.00001);
    if(flag == 1)
        return Euclidean_heuristic;
    else if(flag == 2)
        return Manhattan_heuristic;
    else if(flag == 3)
        return Diagonal_heuristic;
    else    
        return Tie_heuristic;

}

void AstarPathFinder::AstarGraphSearch(Vector3d start_pt, Vector3d end_pt, int flag)
{
    ros::Time time_1 = ros::Time::now();    

    //index of start_point and end_point
    Vector3i start_idx = coord2gridIndex(start_pt);
    Vector3i end_idx   = coord2gridIndex(end_pt);
    goalIdx = end_idx;
    
    ROS_INFO("The cooordinate of the start is %f %f %f", start_pt(0), start_pt(1), start_pt(2));
    ROS_INFO("The cooordinate of the goal is %f %f %f", end_pt(0), end_pt(1), end_pt(2));
    ROS_INFO("The index of the goal is %d %d %d", end_idx(0), end_idx(1), end_idx(2));

    //position of start_point and end_point
    start_pt = gridIndex2coord(start_idx);
    end_pt   = gridIndex2coord(end_idx);

    //Initialize the pointers of struct GridNode which represent start node and goal node
    GridNodePtr startPtr = new GridNode(start_idx, start_pt);
    GridNodePtr endPtr   = new GridNode(end_idx,   end_pt);

    //openSet is the open_list implemented through multimap in STL library
    openSet.clear();

    // currentPtr represents the node with lowest f(n) in the open_list
    GridNodePtr currentPtr  = NULL;
    GridNodePtr neighborPtr = NULL;

    //put start node in open set
    startPtr -> gScore = 0;
    //STEP 1: finish the AstarPathFinder::getHeu , which is the heuristic function
    startPtr -> fScore = startPtr -> gScore + getHeu(startPtr, endPtr, flag);   
    startPtr -> id = 1;  // in openlist
    startPtr -> coord = start_pt; // redundancy
    openSet.insert(make_pair(startPtr -> fScore, startPtr));
    /*
    STEP 2 :  some else preparatory works which should be done before while loop
    please write your code below
    */
    resetUsedGrids();
    GridNodeMap[start_idx(0)][start_idx(1)][start_idx(2)] = startPtr;

    double tentative_gScore;
    vector<GridNodePtr> neighborPtrSets;
    vector<double> edgeCostSets;

    // this is the main loop
    while (!openSet.empty())
    {
        /*
        step 3: Remove the node with lowest cost function from open set to closed set
        please write your code below
        
        This part you should use the C++ STL: multimap, more details can be find in Homework description
        */
        currentPtr = openSet.begin()->second;
        openSet.erase(openSet.begin());
        currentPtr->id = -1; // in closelist

        // if the current node is the goal 
        if( currentPtr->index == goalIdx )
        {
            ros::Time time_2 = ros::Time::now();
            terminatePtr = currentPtr;
            ROS_WARN("[A*]{sucess}  Time in A*  is %f ms, path cost is %f m", (time_2 - time_1).toSec() * 1000.0, currentPtr->gScore * resolution );            
            return;
        }
        //get the successtion
        //STEP 4: finish AstarPathFinder::AstarGetSucc yourself
        AstarGetSucc(currentPtr, neighborPtrSets, edgeCostSets);       

        /*
        STEP 5:  For all unexpanded neigbors "m" of node "n", please finish this for loop
        please write your code below
        *        
        */         
        for(int i = 0; i < (int)neighborPtrSets.size(); i++)
        {
            /*
            Judge if the neigbors have been expanded please write your code below
            
            IMPORTANT NOTE!!!
            neighborPtrSets[i]->id = -1 : expanded, equal to this node is in close set
            neighborPtrSets[i]->id = 1 : unexpanded, equal to this node is in open set        
            */
            tentative_gScore = currentPtr->gScore + edgeCostSets[i];
            neighborPtr = neighborPtrSets[i];
            if(neighborPtr -> id == 0)
            { //discover a new node, which is not in the closed set and open set
                /*
                STEP 6:  As for a new node, do what you need do ,and then put neighbor in open set and record it
                please write your code below
                *        
                */
                neighborPtr -> cameFrom = currentPtr;
                neighborPtr -> gScore = tentative_gScore;
                neighborPtr -> fScore = neighborPtr -> gScore + getHeu(neighborPtr, endPtr, flag);
                neighborPtr -> id = 1;
                openSet.insert(make_pair(neighborPtr -> fScore, neighborPtr));
            }
            else if(neighborPtr -> id == 1)
            {
                //this node is in open set and need to judge if it needs to update
                /*
                STEP 7:  As for a node in open set, update it , maintain the openset, 
                and then put neighbor in open set and record it   
                */
                if(neighborPtr -> gScore > tentative_gScore)
                {
                    std::multimap<double, GridNodePtr>::iterator delete_element;
                    auto iter = openSet.equal_range(neighborPtr -> fScore);
                    if(iter.first != std::end(openSet))
                    {
                        for(auto it = iter.first; it != iter.second; ++it)
                        {
                            if(it->second == neighborPtr)
                            {
                                delete_element = it;
                                break;
                            }
                        }
                    }
                    openSet.erase(delete_element);

                    neighborPtr -> cameFrom = currentPtr;
                    neighborPtr -> gScore = tentative_gScore;
                    neighborPtr -> fScore = neighborPtr -> gScore + getHeu(neighborPtr, endPtr, flag);
                    openSet.insert(make_pair(neighborPtr -> fScore, neighborPtr));
                }
            }
            else
            {
                //this node is in closed set
                if(neighborPtr -> gScore > tentative_gScore)
                {
                    ROS_INFO("Strange!");
                    neighborPtr -> cameFrom = currentPtr;
                    neighborPtr -> gScore = tentative_gScore;
                    neighborPtr -> fScore = neighborPtr -> gScore + getHeu(neighborPtr, endPtr, flag);
                }
            }
        }      
    }
    //if search fails
    ros::Time time_2 = ros::Time::now();
    if((time_2 - time_1).toSec() > 0.1)
        ROS_WARN( "Time consume in Astar path finding is %f", (time_2 - time_1).toSec() );
}


vector<Vector3d> AstarPathFinder::getPath() 
{   
    vector<Vector3d> path;
    vector<GridNodePtr> gridPath;
    /*
    STEP 8:  trace back from the curretnt nodePtr to get all nodes along the path
    please write your code below  
    */
    GridNodePtr currentPtr = terminatePtr;
    while(currentPtr -> cameFrom != NULL)
    {
        gridPath.push_back(currentPtr);
        currentPtr = currentPtr -> cameFrom;
    }
    gridPath.push_back(currentPtr);

    for(auto ptr: gridPath)
        path.push_back(ptr->coord);
        
    reverse(path.begin(),path.end());

    return path;
}