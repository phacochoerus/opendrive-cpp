#include "opendrive-cpp/parser/map_parser.h"

namespace opendrive {
namespace parser {

opendrive::Status MapXmlParser::Parse(const tinyxml2::XMLElement* xml_map,
                                      element::Map::Ptr ele_map) {
  xml_map_ = xml_map;
  ele_map_ = ele_map;
  if (!xml_map_ || !ele_map_) {
    set_status(ErrorCode::XML_ROAD_ELEMENT_ERROR, "Input is null.");
    return status();
  }
  HeaderElement().JunctionElement().RoadElement();
  return status();
}

MapXmlParser& MapXmlParser::HeaderElement() {
  if (!IsValid()) return *this;
  // eq 1
  const tinyxml2::XMLElement* xml_header =
      xml_map_->FirstChildElement("header");
  if (!xml_header) {
    set_status(ErrorCode::XML_HEADER_ELEMENT_ERROR, "HEADER ELEMENT IS NULL.");
    return *this;
  }
  common::XmlQueryStringAttribute(
      xml_header, "revMajor", ele_map_->mutable_header().mutable_rev_major());
  common::XmlQueryStringAttribute(
      xml_header, "revMinor", ele_map_->mutable_header().mutable_rev_minor());
  common::XmlQueryStringAttribute(xml_header, "name",
                                  ele_map_->mutable_header().mutable_name());
  common::XmlQueryStringAttribute(xml_header, "version",
                                  ele_map_->mutable_header().mutable_version());
  common::XmlQueryStringAttribute(xml_header, "vendor",
                                  ele_map_->mutable_header().mutable_vendor());
  common::XmlQueryStringAttribute(xml_header, "date",
                                  ele_map_->mutable_header().mutable_date());
  common::XmlQueryDoubleAttribute(xml_header, "north",
                                  ele_map_->mutable_header().mutable_north());
  common::XmlQueryDoubleAttribute(xml_header, "south",
                                  ele_map_->mutable_header().mutable_south());
  common::XmlQueryDoubleAttribute(xml_header, "west",
                                  ele_map_->mutable_header().mutable_west());
  common::XmlQueryDoubleAttribute(xml_header, "east",
                                  ele_map_->mutable_header().mutable_east());
  this->set_opendrive_version(ele_map_->header().rev_major() + "_" +
                              ele_map_->header().rev_minor() + "_" +
                              ele_map_->header().version());
  return *this;
}

MapXmlParser& MapXmlParser::JunctionElement() {
  if (!IsValid()) return *this;
  // 0~*
  const tinyxml2::XMLElement* curr_xml_junction =
      xml_map_->FirstChildElement("junction");
  Status status{ErrorCode::OK, "ok"};
  while (curr_xml_junction) {
    element::Junction ele_junction;
    common::XmlQueryIntAttribute(curr_xml_junction, "id",
                                 ele_junction.mutable_attribute().mutable_id());
    common::XmlQueryIntAttribute(
        curr_xml_junction, "mainRoad",
        ele_junction.mutable_attribute().mutable_main_road());
    common::XmlQueryDoubleAttribute(
        curr_xml_junction, "sStart",
        ele_junction.mutable_attribute().mutable_start_position());
    common::XmlQueryDoubleAttribute(
        curr_xml_junction, "sEnd",
        ele_junction.mutable_attribute().mutable_end_position());
    common::XmlQueryStringAttribute(
        curr_xml_junction, "name",
        ele_junction.mutable_attribute().mutable_name());
    common::XmlQueryEnumAttribute(
        curr_xml_junction, "orientation",
        ele_junction.mutable_attribute().mutable_dir(), DIR_CHOICES);
    common::XmlQueryEnumAttribute(
        curr_xml_junction, "type",
        ele_junction.mutable_attribute().mutable_type(), JUNCTION_TYPE_CHOICES);
    // junction connection
    // 1~*
    const tinyxml2::XMLElement* curr_xml_connection =
        curr_xml_junction->FirstChildElement("connection");
    if (!curr_xml_connection) {
      set_status(ErrorCode::XML_JUNCTION_CONNECTION_ELEMENT_ERROR,
                 "JUNCTION CONNECTION ELEMENT IS NULL.");
      return *this;
    }
    while (curr_xml_connection) {
      element::JunctionConnection connection;
      common::XmlQueryIntAttribute(curr_xml_connection, "id",
                                   connection.mutable_id());
      common::XmlQueryEnumAttribute(curr_xml_connection, "type",
                                    connection.mutable_type(),
                                    JUNCTION_CONNECTION_TYPE_CHOICES);
      common::XmlQueryIntAttribute(curr_xml_connection, "linkedRoad",
                                   connection.mutable_linked_road());
      common::XmlQueryIntAttribute(curr_xml_connection, "incomingRoad",
                                   connection.mutable_incoming_road());
      common::XmlQueryIntAttribute(curr_xml_connection, "connectingRoad",
                                   connection.mutable_connecting_road());
      common::XmlQueryEnumAttribute(curr_xml_connection, "contactPoint",
                                    connection.mutable_contact_point(),
                                    CONTACT_POINT_TYPE_CHOICES);
      // connection link
      // 0~*
      const tinyxml2::XMLElement* curr_xml_laneLink =
          curr_xml_connection->FirstChildElement("laneLink");
      while (curr_xml_laneLink) {
        element::JunctionLaneLink lane_link;
        common::XmlQueryIntAttribute(curr_xml_laneLink, "from",
                                     lane_link.mutable_from());
        common::XmlQueryIntAttribute(curr_xml_laneLink, "to",
                                     lane_link.mutable_to());
        connection.mutable_lane_links().emplace_back(lane_link);
        curr_xml_laneLink = common::XmlNextSiblingElement(curr_xml_laneLink);
      }
      ele_junction.mutable_connections().emplace_back(connection);
      curr_xml_connection = common::XmlNextSiblingElement(curr_xml_connection);
    }
    ele_map_->mutable_junctions().emplace_back(ele_junction);
    curr_xml_junction = common::XmlNextSiblingElement(curr_xml_junction);
  }
  return *this;
}

MapXmlParser& MapXmlParser::RoadElement() {
  if (!IsValid()) return *this;
  // 1~*
  const tinyxml2::XMLElement* curr_xml_road =
      xml_map_->FirstChildElement("road");
  if (!curr_xml_road) {
    set_status(ErrorCode::XML_ROAD_ELEMENT_ERROR, "ROAD ELEMENT IS NULL.");
    return *this;
  }
  RoadXmlParser road_parser{this->opendrive_version()};
  Status status{ErrorCode::OK, "ok"};
  while (curr_xml_road) {
    element::Road ele_road;
    status = road_parser.Parse(curr_xml_road, &ele_road);
    CheckStatus(status);
    ele_map_->mutable_roads().emplace_back(ele_road);
    curr_xml_road = common::XmlNextSiblingElement(curr_xml_road);
  }
  return *this;
}

}  // namespace parser
}  // namespace opendrive
