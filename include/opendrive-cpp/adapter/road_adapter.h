#ifndef OPENDRIVE_CPP_ADAPTER_ROAD_H_
#define OPENDRIVE_CPP_ADAPTER_ROAD_H_

#include <memory>
#include "opendrive-cpp/adapter/geometry_adapter.h"
#include "opendrive-cpp/adapter/lane_adapter.h"
#include "opendrive-cpp/adapter/util_adapter.h"
#include "opendrive-cpp/geometry/core.h"

namespace opendrive {
namespace adapter {

class RoadAdapter : public UtilAdapter {
 public:
  RoadAdapter();
  opendrive::Status Run(const g::Road* g_road, core::Road::Ptr road_ptr);

 private:
  virtual void Init() override;
  RoadAdapter& TransformGeometry();
  RoadAdapter& TransformSection();
  std::shared_ptr<GeometryAdapter> geometry_ptr_;
  const g::Road* g_road_;
  core::Road::Ptr road_ptr_;
};

}  // namespace adapter
}  // namespace opendrive

#endif  // OPENDRIVE_CPP_ADAPTER_ROAD_H_
