#ifndef OPENDRIVE_CPP_LANES_XML_PARSER_H_
#define OPENDRIVE_CPP_LANES_XML_PARSER_H_

#include "opendrive-cpp/common/common.hpp"
#include "opendrive-cpp/common/status.h"
#include "opendrive-cpp/core/types.hpp"
#include "parser.h"

namespace opendrive {
namespace parser {

class LanesXmlParser : public XmlParser {
 public:
  LanesXmlParser() = default;
  opendrive::Status Parse(const tinyxml2::XMLElement* lanes_ele,
                          base::Lanes::Ptr lanes_ptr);

 private:
  virtual void Init() override;
  LanesXmlParser& ParseLaneOffsetEle();
  LanesXmlParser& ParseLaneSectionEle();
  LanesXmlParser& ParseLaneSectionLanesEle(
      const tinyxml2::XMLElement* sections_ele,
      base::LaneSection& lane_section);

  LanesXmlParser& ParseLaneEle(const tinyxml2::XMLElement* lane_ele,
                               base::Lane& lane);
  LanesXmlParser& ParseLaneAttribute(const tinyxml2::XMLElement* lane_ele,
                                     base::Lane& lane);
  LanesXmlParser& ParseLaneLinkEle(const tinyxml2::XMLElement* lane_ele,
                                   base::Lane& lane);
  LanesXmlParser& ParseLaneWidthEle(const tinyxml2::XMLElement* lane_ele,
                                    base::Lane& lane);
  LanesXmlParser& ParseLaneBorderEle(const tinyxml2::XMLElement* lane_ele,
                                     base::Lane& lane);
  LanesXmlParser& ParseLaneRoadMarkEle(const tinyxml2::XMLElement* lane_ele,
                                       base::Lane& lane);

  const tinyxml2::XMLElement* lanes_ele_;
  base::Lanes::Ptr lanes_ptr_;
};
}  // namespace parser
}  // namespace opendrive

#endif  // OPENDRIVE_CPP_LANES_XML_PARSER_H_
