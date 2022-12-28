#ifndef OPENDRIVE_CPP_CORE_H_
#define OPENDRIVE_CPP_CORE_H_

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "opendrive-cpp/geometry/element.h"
#include "opendrive-cpp/geometry/enums.h"

namespace opendrive {
namespace core {

typedef std::string Id;
typedef std::unordered_set<Id> Ids;
typedef std::vector<Id> Path;

struct Point2D {
  double x = 0.;
  double y = 0.;
};
typedef Point2D Point;
typedef std::vector<Point> Points;
typedef std::vector<Point2D> Point2Ds;

typedef struct Lane LaneTypedef;
struct Lane {
  struct Point : public Point2D {
    Id id;
    double s = 0.;
    double hdg = 0.;
    Id parent;  // lane id
  };
  typedef std::vector<Point> Pts;
  typedef std::vector<Point> Points;
  struct Boundary {
    Points line;
    RoadMarkType type = RoadMarkType::UNKNOWN;
    RoadMarkColor color = RoadMarkColor::UNKNOWN;
  };
  typedef std::shared_ptr<LaneTypedef> Ptr;
  typedef std::shared_ptr<LaneTypedef const> ConstPtr;
  typedef std::vector<Ptr> Ptrs;
  Id id;      // [required]
  Id parent;  // section id
  LaneType type = LaneType::UNKNOWN;
  double length = 0.;
  // TODO: road and lane speed limit 待处理
  float speed_limit = -1;
  Points central_curve;
  Boundary left_boundary;
  Boundary right_boundary;
  Ids predecessors;
  Ids successors;
  Id left_neighbor;
  Id right_neighbor;
};

typedef struct Section SectionTypedef;
struct Section {
  typedef std::shared_ptr<SectionTypedef> Ptr;
  typedef std::vector<Ptr> Ptrs;
  typedef std::shared_ptr<SectionTypedef const> ConstPtr;
  Id id;      // [required]
  Id parent;  // road id
  double s = 0.;
  double length = 0.;
  Lane::Ptr center_lane;
  Lane::Ptrs left_lanes;
  Lane::Ptrs right_lanes;
};

typedef struct Road RoadTypedef;
struct Road {
  typedef std::shared_ptr<RoadTypedef> Ptr;
  typedef std::shared_ptr<RoadTypedef const> ConstPtr;
  Id id;  // [required]
  double length = 0.;
  Section::Ptrs sections;
  Id predecessor;
  Id successor;
  RoadLinkType predecessor_type = RoadLinkType::UNKNOWN;
  RoadLinkType successor_type = RoadLinkType::UNKNOWN;
  ContactPointType predecessor_contact_point = ContactPointType::UNKNOWN;
  ContactPointType successor_contact_point = ContactPointType::UNKNOWN;
};

typedef struct Junction JunctionTypedef;
struct Junction {
  struct Connection {
    Id id;       // [required]
    Id link_id;  // [required] [extended] (incoming_road + "_" +
                 // connecting_road)
    JunctionConnectionType type = JunctionConnectionType::UNKNOWN;
    Id incoming_road;
    Id connecting_road;
    ContactPointType contact_point = ContactPointType::UNKNOWN;
    std::vector<std::pair<Id, Id>> lane_links;  // {from : to}
  };
  typedef std::unordered_map<core::Id, Connection>
      Connections;  // e.g. {"1_2": conn}
  typedef std::shared_ptr<JunctionTypedef> Ptr;
  typedef std::shared_ptr<JunctionTypedef const> ConstPtr;
  Id id;
  std::string name;
  Id main_road;                    // virtual junctions v1.7
  double s_start;                  // virtual junctions v1.7
  double s_end;                    // virtual junctions v1.7
  Dir orientation = Dir::UNKNOWN;  // virtual junctions v1.7
  JunctionType type = JunctionType::DEFAULT;
  Connections connections;
};

typedef struct Header HeaderTypedef;
struct Header {
  typedef std::shared_ptr<HeaderTypedef> Ptr;
  std::string rev_major;
  std::string rev_minor;
  std::string name;
  std::string version;
  std::string date;
  double north = 0.;
  double south = 0.;
  double west = 0.;
  double east = 0.;
  std::string vendor;
};

typedef struct Map MapTypedef;
struct Map {
  typedef std::shared_ptr<MapTypedef> Ptr;
  Header::Ptr header;
  std::unordered_map<core::Id, Road::Ptr> roads;
  std::unordered_map<core::Id, Junction::Ptr> junctions;
};

}  // namespace core
}  // namespace opendrive

#endif  // OPENDRIVE_CPP_CORE_H_
