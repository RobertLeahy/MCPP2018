#include <mcpp/yggdrasil/authenticate.hpp>

#include <string>
#include <string_view>
#include <mcpp/yggdrasil/profile.hpp>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (authenticate_request)",
          "[mcpp][yggdrasil][authenticate][authenticate_request][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  authenticate_request auth;
  auth.agent.name = "foo";
  auth.agent.version = 2;
  auth.username = "bar";
  auth.password = "quux";
  SECTION("No clientToken") {
    SECTION("requestUser = false") {
      auto result = to_json(auth,
                            w);
      REQUIRE(result);
      std::string_view str(sb.GetString(),
                           sb.GetSize());
      std::string_view expected("{\"agent\":{\"name\":\"foo\","
                                            "\"version\":2},"
                                 "\"username\":\"bar\","
                                 "\"password\":\"quux\"}");
      CHECK(str == expected);
    }
    SECTION("requestUser = true") {
      auth.request_user = true;
      auto result = to_json(auth,
                            w);
      REQUIRE(result);
      std::string_view str(sb.GetString(),
                           sb.GetSize());
      std::string_view expected("{\"agent\":{\"name\":\"foo\","
                                            "\"version\":2},"
                                 "\"username\":\"bar\","
                                 "\"password\":\"quux\","
                                 "\"requestUser\":true}");
      CHECK(str == expected);
    }
  }
  SECTION("clientToken") {
    auth.client_token = "corge";
    SECTION("requestUser = false") {
      auto result = to_json(auth,
                            w);
      REQUIRE(result);
      std::string_view str(sb.GetString(),
                           sb.GetSize());
      std::string_view expected("{\"agent\":{\"name\":\"foo\","
                                            "\"version\":2},"
                                 "\"username\":\"bar\","
                                 "\"password\":\"quux\","
                                 "\"clientToken\":\"corge\"}");
      CHECK(str == expected);
    }
    SECTION("requestUser = true") {
      auth.request_user = true;
      auto result = to_json(auth,
                            w);
      REQUIRE(result);
      std::string_view str(sb.GetString(),
                           sb.GetSize());
      std::string_view expected("{\"agent\":{\"name\":\"foo\","
                                            "\"version\":2},"
                                 "\"username\":\"bar\","
                                 "\"password\":\"quux\","
                                 "\"clientToken\":\"corge\","
                                 "\"requestUser\":true}");
      CHECK(str == expected);
    }
  }
}

TEST_CASE("authenticate_request_parser",
          "[mcpp][yggdrasil][authenticate][authenticate_request][json][parse]")
{
  authenticate_request auth;
  authenticate_request_parser handler(auth);
  ::rapidjson::Reader reader;
  SECTION("Empty object") {
    ::rapidjson::StringStream ss("{}");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Not an object") {
    ::rapidjson::StringStream ss("5");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Good (clientToken & requestUser = true)") {
    ::rapidjson::StringStream ss("{\"agent\":{\"name\":\"foo\","
                                             "\"version\":2},"
                                  "\"username\":\"bar\","
                                  "\"password\":\"quux\","
                                  "\"clientToken\":\"corge\","
                                  "\"requestUser\":true}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.agent.name == "foo");
    CHECK(auth.agent.version == 2);
    CHECK(auth.username == "bar");
    CHECK(auth.password == "quux");
    CHECK(auth.request_user);
    REQUIRE(auth.client_token);
    CHECK(*auth.client_token == "corge");
  }
  SECTION("Good (requestUser = true)") {
    ::rapidjson::StringStream ss("{\"agent\":{\"name\":\"foo\","
                                             "\"version\":2},"
                                  "\"username\":\"bar\","
                                  "\"password\":\"quux\","
                                  "\"requestUser\":true}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.agent.name == "foo");
    CHECK(auth.agent.version == 2);
    CHECK(auth.username == "bar");
    CHECK(auth.password == "quux");
    CHECK(auth.request_user);
    CHECK_FALSE(auth.client_token);
  }
  SECTION("Good (clientToken)") {
    ::rapidjson::StringStream ss("{\"agent\":{\"name\":\"foo\","
                                             "\"version\":2},"
                                  "\"username\":\"bar\","
                                  "\"password\":\"quux\","
                                  "\"clientToken\":\"corge\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.agent.name == "foo");
    CHECK(auth.agent.version == 2);
    CHECK(auth.username == "bar");
    CHECK(auth.password == "quux");
    CHECK_FALSE(auth.request_user);
    REQUIRE(auth.client_token);
    CHECK(*auth.client_token == "corge");
  }
  SECTION("Good") {
    ::rapidjson::StringStream ss("{\"agent\":{\"name\":\"foo\","
                                             "\"version\":2},"
                                  "\"username\":\"bar\","
                                  "\"password\":\"quux\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.agent.name == "foo");
    CHECK(auth.agent.version == 2);
    CHECK(auth.username == "bar");
    CHECK(auth.password == "quux");
    CHECK_FALSE(auth.request_user);
    CHECK_FALSE(auth.client_token);
  }
  SECTION("clear") {
    ::rapidjson::StringStream ss("{\"agent\":{\"name\":\"foo\","
                                             "\"version\":2},"
                                  "\"username\":\"bar\","
                                  "\"password\":\"quux\","
                                  "\"clientToken\":\"corge\","
                                  "\"requestUser\":true}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.client_token);
    CHECK(auth.request_user);
    handler.clear();
    ::rapidjson::StringStream ss2("{\"agent\":{\"name\":\"foo\","
                                              "\"version\":2},"
                                   "\"username\":\"bar\","
                                   "\"password\":\"quux\"}");
    result = reader.Parse(ss2,
                          handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(auth.agent.name == "foo");
    CHECK(auth.agent.version == 2);
    CHECK(auth.username == "bar");
    CHECK(auth.password == "quux");
    CHECK_FALSE(auth.request_user);
    CHECK_FALSE(auth.client_token);
  }
}

TEST_CASE("to_json (authenticate_response)",
          "[mcpp][yggdrasil][authenticate][authenticate_response][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  authenticate_response auth;
  auth.access_token = "foo";
  auth.client_token = "bar";
  SECTION("Tokens only") {
    auto result = to_json(auth,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"accessToken\":\"foo\","
                               "\"clientToken\":\"bar\"}");
    CHECK(str == expected);
  }
  SECTION("Everything (no available profiles)") {
    auth.available_profiles.emplace();
    auth.selected_profile.emplace();
    auth.selected_profile->id = "quux";
    auth.selected_profile->name = "baz";
    auth.user.emplace();
    auth.user->id = "corge";
    auto result = to_json(auth,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"accessToken\":\"foo\","
                               "\"clientToken\":\"bar\","
                               "\"availableProfiles\":[],"
                               "\"selectedProfile\":{\"id\":\"quux\","
                                                    "\"name\":\"baz\"},"
                               "\"user\":{\"id\":\"corge\","
                                         "\"properties\":[]}}");
    CHECK(str == expected);
  }
  SECTION("Everything (available profiles)") {
    auth.available_profiles.emplace();
    profile p;
    p.id = "barbaz";
    p.name = "quuxcorge";
    auth.available_profiles->push_back(p);
    auth.selected_profile.emplace();
    auth.selected_profile->id = "quux";
    auth.selected_profile->name = "baz";
    auth.user.emplace();
    auth.user->id = "corge";
    auto result = to_json(auth,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"accessToken\":\"foo\","
                               "\"clientToken\":\"bar\","
                               "\"availableProfiles\":[{\"id\":\"barbaz\","
                                                       "\"name\":\"quuxcorge\"}],"
                               "\"selectedProfile\":{\"id\":\"quux\","
                                                    "\"name\":\"baz\"},"
                               "\"user\":{\"id\":\"corge\","
                                         "\"properties\":[]}}");
    CHECK(str == expected);
  }
}

TEST_CASE("authenticate_response_parser",
          "[mcpp][yggdrasil][authenticate][authenticate_response][json][parse]")
{
  authenticate_response auth;
  authenticate_response_parser handler(auth);
  ::rapidjson::Reader reader;
  SECTION("Empty object") {
    ::rapidjson::StringStream ss("{}");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Not an object") {
    ::rapidjson::StringStream ss("5");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Tokens only") {
    ::rapidjson::StringStream ss("{\"accessToken\":\"foo\","
                                  "\"clientToken\":\"bar\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    auto ec = handler.error_code();
    REQUIRE_FALSE(ec);
    CHECK(handler.done());
    CHECK(auth.access_token == "foo");
    CHECK(auth.client_token == "bar");
    CHECK_FALSE(auth.available_profiles);
    CHECK_FALSE(auth.selected_profile);
    CHECK_FALSE(auth.user);
  }
  SECTION("Everything (available profiles") {
    ::rapidjson::StringStream ss("{\"accessToken\":\"foo\","
                                  "\"clientToken\":\"bar\","
                                  "\"availableProfiles\":[{\"id\":\"barbaz\","
                                                          "\"name\":\"quuxcorge\"}],"
                                  "\"selectedProfile\":{\"id\":\"quux\","
                                                       "\"name\":\"baz\"},"
                                  "\"user\":{\"id\":\"corge\","
                                            "\"properties\":[]}}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    auto ec = handler.error_code();
    REQUIRE_FALSE(ec);
    CHECK(handler.done());
    CHECK(auth.access_token == "foo");
    CHECK(auth.client_token == "bar");
    REQUIRE(auth.available_profiles);
    auto iter = auth.available_profiles->begin();
    REQUIRE_FALSE(iter == auth.available_profiles->end());
    CHECK(iter->id == "barbaz");
    CHECK(iter->name == "quuxcorge");
    ++iter;
    CHECK(iter == auth.available_profiles->end());
    REQUIRE(auth.selected_profile);
    CHECK(auth.selected_profile->id == "quux");
    CHECK(auth.selected_profile->name == "baz");
    REQUIRE(auth.user);
    CHECK(auth.user->id == "corge");
    CHECK(auth.user->properties.empty());
  }
  SECTION("Everything (no available profiles)") {
    ::rapidjson::StringStream ss("{\"accessToken\":\"foo\","
                                  "\"clientToken\":\"bar\","
                                  "\"availableProfiles\":[],"
                                  "\"selectedProfile\":{\"id\":\"quux\","
                                                       "\"name\":\"baz\"},"
                                  "\"user\":{\"id\":\"corge\","
                                            "\"properties\":[]}}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    auto ec = handler.error_code();
    REQUIRE_FALSE(ec);
    CHECK(handler.done());
    CHECK(auth.access_token == "foo");
    CHECK(auth.client_token == "bar");
    REQUIRE(auth.available_profiles);
    CHECK(auth.available_profiles->empty());
    REQUIRE(auth.selected_profile);
    CHECK(auth.selected_profile->id == "quux");
    CHECK(auth.selected_profile->name == "baz");
    REQUIRE(auth.user);
    CHECK(auth.user->id == "corge");
    CHECK(auth.user->properties.empty());
  }
  SECTION("clear") {
    ::rapidjson::StringStream ss("{\"accessToken\":\"foo\","
                                  "\"clientToken\":\"bar\","
                                  "\"availableProfiles\":[{\"id\":\"barbaz\","
                                                          "\"name\":\"quuxcorge\"}],"
                                  "\"selectedProfile\":{\"id\":\"quux\","
                                                       "\"name\":\"baz\"},"
                                  "\"user\":{\"id\":\"corge\","
                                            "\"properties\":[]}}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    auto ec = handler.error_code();
    REQUIRE_FALSE(ec);
    CHECK(handler.done());
    handler.clear();
    ::rapidjson::StringStream ss2("{\"accessToken\":\"foo\","
                                   "\"clientToken\":\"bar\","
                                   "\"availableProfiles\":[],"
                                   "\"selectedProfile\":{\"id\":\"quux\","
                                                        "\"name\":\"baz\"},"
                                   "\"user\":{\"id\":\"corge\","
                                             "\"properties\":[]}}");
    result = reader.Parse(ss2,
                          handler);
    REQUIRE(result);
    ec = handler.error_code();
    REQUIRE_FALSE(ec);
    CHECK(handler.done());
    CHECK(auth.access_token == "foo");
    CHECK(auth.client_token == "bar");
    REQUIRE(auth.available_profiles);
    CHECK(auth.available_profiles->empty());
    REQUIRE(auth.selected_profile);
    CHECK(auth.selected_profile->id == "quux");
    CHECK(auth.selected_profile->name == "baz");
    REQUIRE(auth.user);
    CHECK(auth.user->id == "corge");
    CHECK(auth.user->properties.empty());
  }
}

}
}
