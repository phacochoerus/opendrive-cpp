#ifndef OPENDRIVE_CPP_TYPES_H_
#define OPENDRIVE_CPP_TYPES_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "enums.h"
#include "opendrive-cpp/common/common.hpp"
#include "opendrive-cpp/common/spiral/odrSpiral.h"

namespace opendrive {
namespace element {

typedef int Id;
typedef std::vector<Id> Ids;
typedef std::string IdStr;
typedef std::string Name;
typedef std::array<double, 1> Vec1D;
typedef std::array<double, 2> Vec2D;
typedef std::array<double, 3> Vec3D;

struct Header {
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

typedef struct Geometry GeometryTypedef;
class Geometry {
 public:
  typedef std::shared_ptr<GeometryTypedef> Ptr;
  Geometry(double _s, double _x, double _y, double _hdg, double _length,
           GeometryType _type)
      : s(_s),
        x(_x),
        y(_y),
        hdg(_hdg),
        length(_length),
        type(_type),
        sin_hdg(std::sin(_hdg)),
        cos_hdg(std::cos(_hdg)) {}
  virtual ~Geometry() = default;
  virtual Vec3D GetXYH(double distance) = 0;
  const double s;
  const double x;
  const double y;
  const double hdg;
  const double length;
  const GeometryType type;
  const double sin_hdg;
  const double cos_hdg;
};

class GeometryLine final : public Geometry {
 public:
  GeometryLine(double _s, double _x, double _y, double _hdg, double _length,
               GeometryType _type)
      : Geometry(_s, _x, _y, _hdg, _length, _type) {}
  virtual Vec3D GetXYH(double distance) override {
    distance = common::Clamp<double>(distance, 0, length);
    const double xd = x + (cos_hdg * (distance - s));
    const double yd = y + (sin_hdg * (distance - s));
    return Vec3D{xd, yd, hdg};
  }
};

class GeometryArc final : public Geometry {
 public:
  GeometryArc(double _s, double _x, double _y, double _hdg, double _length,
              GeometryType _type, double _curvature)
      : Geometry(_s, _x, _y, _hdg, _length, _type),
        curvature(_curvature),
        radius(1.0 / _curvature) {}
  virtual Vec3D GetXYH(double distance) override {
    distance = common::Clamp<double>(distance, 0, length);
    const double angle_at_s = (distance - s) * curvature - M_PI / 2;
    const double xd = radius * (std::cos(hdg + angle_at_s) - sin_hdg) + x;
    const double yd = radius * (std::sin(hdg + angle_at_s) + cos_hdg) + y;
    const double tangent = hdg + distance * curvature;
    return Vec3D{xd, yd, tangent};
  }
  const double curvature;
  const double radius;
};

class GeometrySpiral final : public Geometry {
 public:
  GeometrySpiral(double _s, double _x, double _y, double _hdg, double _length,
                 GeometryType _type, double _curve_start, double _curve_end)
      : Geometry(_s, _x, _y, _hdg, _length, _type),
        curve_start(_curve_start),
        curve_end(_curve_end),
        curve_dot((_curve_end - _curve_start) / (_length)) {}

  virtual Vec3D GetXYH(double distance) override {
    distance = common::Clamp<double>(distance, 0, length);

    const double s1 = curve_start / curve_dot + distance;
    double x1;
    double y1;
    double t1;
    odrSpiral(s1, curve_dot, &x1, &y1, &t1);

    const double s0 = curve_start / curve_dot;
    double x0;
    double y0;
    double t0;
    odrSpiral(s1, curve_dot, &x0, &y0, &t0);

    x1 -= x0;
    y1 -= y0;
    t1 -= t0;
    const double angle = hdg - t0;
    const double cos_a = std::cos(angle);
    const double sin_a = std::sin(angle);
    const double xd = x + x1 * cos_a - y1 * sin_a;
    const double yd = y + y1 * cos_a + x1 * sin_a;
    const double tangent = hdg + t1;
    return Vec3D{xd, yd, tangent};
  }
  const double curve_start;
  const double curve_end;
  const double curve_dot;
};

class GeometryPoly3 final : public Geometry {
 public:
  GeometryPoly3(double _s, double _x, double _y, double _hdg, double _length,
                GeometryType _type, double _a, double _b, double _c, double _d)
      : Geometry(_s, _x, _y, _hdg, _length, _type),
        a(_a),
        b(_b),
        c(_c),
        d(_d) {}

  virtual Vec3D GetXYH(double distance) override {
    distance = common::Clamp<double>(distance, 0, length);
    const double u = distance;
    const double v = a + b * u + c * std::pow(u, 2) + d * std::pow(u, 3);
    const double x1 = u * cos_hdg - v * sin_hdg;
    const double y1 = u * sin_hdg + v * cos_hdg;
    const double tangent_v = b + 2.0 * c * u + 3.0 * d * std::pow(u, 2);
    const double theta = std::atan2(tangent_v, 1.0);
    const double xd = x + x1;
    const double yd = y + y1;
    const double tangent = hdg + theta;
    return Vec3D{xd, yd, tangent};
  }
  const double a;
  const double b;
  const double c;
  const double d;
};

class GeometryParamPoly3 final : public Geometry {
 public:
  enum class PRange : std::uint8_t {
    UNKNOWN = 0,
    ARCLENGTH = 1,
    NORMALIZED = 2
  };
  GeometryParamPoly3(double _s, double _x, double _y, double _hdg,
                     double _length, GeometryType _type, double _au, double _bu,
                     double _cu, double _du, double _av, double _bv, double _cv,
                     double _dv, PRange _p_range)
      : Geometry(_s, _x, _y, _hdg, _length, _type),
        au(_au),
        bu(_bu),
        cu(_cu),
        du(_du),
        av(_av),
        bv(_bv),
        cv(_cv),
        dv(_dv),
        p_range(_p_range) {}
  virtual Vec3D GetXYH(double distance) override {
    distance = common::Clamp<double>(distance, 0, length);
    double p = distance;
    if (PRange::NORMALIZED == p_range) {
      p = std::min(1.0, distance / length);
    }
    const double u = au + bu * p + cu * std::pow(p, 2) + du * std::pow(p, 3);
    const double v = av + bv * p + cv * std::pow(p, 2) + dv * std::pow(p, 3);
    const double x1 = u * cos_hdg - v * sin_hdg;
    const double y1 = u * sin_hdg + v * cos_hdg;
    const double tangent_u = bu + 2 * cu * p + 3 * du * std::pow(p, 2);
    const double tangent_v = bv + 2 * cv * p + 3 * dv * std::pow(p, 2);
    const double theta = std::atan2(tangent_v, tangent_u);
    const double xd = x + x1;
    const double yd = y + y1;
    const double tangent = hdg + theta;
    return Vec3D{xd, yd, tangent};
  }
  const PRange p_range;
  const double au;
  const double bu;
  const double cu;
  const double du;
  const double av;
  const double bv;
  const double cv;
  const double dv;
};

struct LaneAttributes {
  Id id = -1;
  LaneType type = LaneType::UNKNOWN;
  Boolean level = Boolean::UNKNOWN;
  double length = 0.;  // lane length(extended)
};

struct OffsetPoly3 {
  // f(s) = a + b*s + c*s*s + d*s*s*s
  double s = 0.;  // start position (s - start_offset)[meters]]
  double a = 0.;  // a - polynomial value at start_offset=0
  double b = 0.;  // b
  double c = 0.;  // c
  double d = 0.;  // d
  bool operator<(const OffsetPoly3 p) const { return s > p.s; }
  virtual double GetOffset(double ds) const final {
    return a + b * ds + c * std::pow(ds, 2) + d * std::pow(ds, 3);
  }
};

struct RoadMark {
  double s = 0.;
  RoadMarkType type = RoadMarkType::UNKNOWN;
  RoadMarkColor color = RoadMarkColor::UNKNOWN;
  RoadMarkWeight weigth = RoadMarkWeight::UNKNOWN;
  double width = 0.;
  double height = 0.;
  std::string material = "standard";
  RoadMarkLaneChange lane_change = RoadMarkLaneChange::UNKNOWN;
};

struct LaneWidth : public OffsetPoly3 {};

struct LaneBorder : public OffsetPoly3 {};

struct LaneLink {
  Id predecessor = 0;
  Id successor = 0;
};

struct Lane {
  LaneAttributes attributes;
  LaneLink link;
  std::vector<LaneWidth> widths;
  std::vector<LaneBorder> borders;
  std::vector<RoadMark> road_marks;
  double GetLaneWidth(double ds) {
    // width >> border
    if (ds < 0) {
      return GetLaneWidth(0);
    }
    if (widths.empty()) {
      if (borders.empty()) {
        return 0.;
      }
      /// border
      for (size_t i = 0; i < borders.size(); i++) {
        if (1 == borders.size() || i == borders.size() - 1) {
          return borders.at(i).GetOffset(ds);
        } else {
          if (ds <= borders.at(i).s) {
            return borders.at(i - 1).GetOffset(ds);
          }
        }
      }
    } else {
      /// width
      for (size_t i = 0; i < widths.size(); i++) {
        if (1 == widths.size() || i == widths.size() - 1) {
          return widths.at(i).GetOffset(ds);
        } else {
          if (ds <= widths.at(i).s) {
            return widths.at(i - 1).GetOffset(ds);
          }
        }
      }
    }
    return 0.;
  }
};

struct LanesInfo {
  std::vector<Lane> lanes;
};

struct LaneOffset : public OffsetPoly3 {};

struct LaneSection {
  double s0 = 0.;  // start position
  double s1 = 0.;  // end position(extended)
  LanesInfo left, center, right;
};

struct Lanes {
  std::vector<LaneOffset> lane_offsets;
  std::vector<LaneSection> lane_sections;
};

struct RoadAttributes {
  Name name;
  Id id = -1;
  Id junction = -1;
  double length = 0.;
  RoadRule rule = RoadRule::UNKNOWN;
};

struct RoadLinkInfo {
  enum class Type { UNKNOWN = 0, ROAD = 1, JUNCTION = 2 };
  enum class PointType { UNKNOWN = 0, START = 1, END = 2 };
  enum class Dir { UNKNOWN = 0, PLUS = 1, MINUS = 2 };
  Id id = -1;
  double s = 0.;
  Type type = Type::UNKNOWN;
  PointType point_type = PointType::UNKNOWN;
  Dir dir = Dir::UNKNOWN;
};

struct RoadLink {
  RoadLinkInfo predecessor;
  RoadLinkInfo successor;
};

struct RoadTypeInfo {
  double s = 0.;
  RoadType type = RoadType::UNKNOWN;
  std::string country;
  double max_speed = 0.;
  RoadSpeedUnit speed_unit = RoadSpeedUnit::UNKNOWN;
};

struct RoadPlanView {
  std::vector<Geometry::Ptr> geometrys;
};

struct Road {
  RoadAttributes attributes;
  RoadLink link;
  std::vector<RoadTypeInfo> type_info;
  RoadPlanView plan_view;
  Lanes lanes;
};

typedef struct Map MapType;
struct Map {
  typedef std::shared_ptr<MapType> Ptr;
  Header header;
  std::vector<Road> roads;
};

}  // namespace element
}  // namespace opendrive

#endif  // OPENDRIVE_CPP_TYPES_H_
