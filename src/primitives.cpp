
#include "primitives.hpp"
#include "moab/GeomUtil.hpp"

#include <cassert>

void DblTriBounds(void* tris_i, size_t item, RTCBounds& bounds_o) {

  const DblTri* tris = (const DblTri*) tris_i;
  const DblTri& this_tri = tris[item];  

  moab::Interface* mbi = (moab::Interface*) this_tri.moab_instance;
  moab::ErrorCode rval;

  std::vector<moab::EntityHandle> conn;
  rval = mbi->get_connectivity(&(this_tri.handle), 1, conn);
  MB_CHK_SET_ERR_CONT(rval, "Failed to get triangle connectivity");

  assert(conn.size() == 3);
  moab::CartVect coords[3];
  rval = mbi->get_coords(&conn[0], 1, coords[0].array());
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");
  rval = mbi->get_coords(&conn[1], 1, coords[1].array());
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");
  rval = mbi->get_coords(&conn[2], 1, coords[2].array());
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");

  double bump_val = 5e-03;
  
  bounds_o.lower_x = std::min(coords[0][0],std::min(coords[1][0],coords[2][0]));
  bounds_o.lower_y = std::min(coords[0][1],std::min(coords[1][1],coords[2][1]));
  bounds_o.lower_z = std::min(coords[0][2],std::min(coords[1][2],coords[2][2]));

  bounds_o.upper_x = std::max(coords[0][0],std::max(coords[1][0],coords[2][0]));
  bounds_o.upper_y = std::max(coords[0][1],std::max(coords[1][1],coords[2][1]));
  bounds_o.upper_z = std::max(coords[0][2],std::max(coords[1][2],coords[2][2]));
  
  bounds_o.lower_x -= bump_val; bounds_o.lower_y -= bump_val; bounds_o.lower_z -= bump_val; 
  bounds_o.upper_x += bump_val; bounds_o.upper_y += bump_val; bounds_o.upper_z += bump_val; 

  return;
}


void DblTriIntersectFunc(void* tris_i, RTCDRay& ray, size_t item) {

  const DblTri* tris = (const DblTri*) tris_i;
  const DblTri& this_tri = tris[item];  

  moab::Interface* mbi = (moab::Interface*) this_tri.moab_instance;
  moab::ErrorCode rval;

  std::vector<moab::EntityHandle> conn;
  rval = mbi->get_connectivity(&(this_tri.handle), 1, conn);
  MB_CHK_SET_ERR_CONT(rval, "Failed to get triangle connectivity");

  moab::CartVect coords[3];
  rval = mbi->get_coords(&conn[0], 1, &(coords[0][0]));
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");
  rval = mbi->get_coords(&conn[1], 1, &(coords[1][0]));
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");
  rval = mbi->get_coords(&conn[2], 1, &(coords[2][0]));
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");
  
  double dist;
  double nonneg_ray_len = 1e17;
  const double* ptr = &nonneg_ray_len;
  moab::CartVect ray_org(ray.dorg);
  moab::CartVect ray_dir(ray.ddir);

  bool hit = moab::GeomUtil::plucker_ray_tri_intersect(coords, ray_org, ray_dir, dist, ptr);
  
  if ( hit ) {
    ray.dtfar = dist;
    ray.tfar = dist;
    ray.u = 0.0f;
    ray.v = 0.0f;
    ray.geomID = this_tri.geomID;
    ray.primID = (unsigned int) item;

    moab::CartVect normal = (coords[1] - coords[0])*(coords[2] - coords[0]);

    if( -1 == this_tri.sense ) normal *= -1;
    
    ray.Ng[0] = normal[0];
    ray.Ng[1] = normal[1];
    ray.Ng[2] = normal[2];


  }

  return;
}

void DblTriOccludedFunc(void* tris_i, RTCDRay& ray, size_t item) {

  const DblTri* tris = (const DblTri*) tris_i;
  const DblTri& this_tri = tris[item];  
  RTCBounds bounds;

  moab::Interface* mbi = (moab::Interface*) this_tri.moab_instance;
  moab::ErrorCode rval;

  std::vector<moab::EntityHandle> conn;
  rval = mbi->get_connectivity(&(this_tri.handle), 1, conn);
  MB_CHK_SET_ERR_CONT(rval, "Failed to get triangle connectivity");

  moab::CartVect coords[3];
  rval = mbi->get_coords(&conn.front(), conn.size(), coords[0].array());
  MB_CHK_SET_ERR_CONT(rval, "Failed to get vertext coordinates");

  double dist;
  double nonneg_ray_len = 1e37;
  double* ptr = &nonneg_ray_len;
  moab::CartVect ray_org(ray.dorg);
  moab::CartVect ray_dir(ray.ddir);
  
  bool hit = moab::GeomUtil::plucker_ray_tri_intersect(coords, ray_org, ray_dir, dist, ptr);
  if ( hit ) {
    ray.geomID = 0;
  }
}