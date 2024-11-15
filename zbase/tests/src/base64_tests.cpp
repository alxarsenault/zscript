#include <catch2.h>
#include <zbase/crypto/base64.h>

TEST_CASE("Base64 : get_encoded_size") {
  REQUIRE(zb::base64::encoded_size(1) == 4);
  REQUIRE(zb::base64::encoded_size(2) == 4);
  REQUIRE(zb::base64::encoded_size(3) == 4);
  REQUIRE(zb::base64::encoded_size(4) == 8);
  REQUIRE(zb::base64::encoded_size(5) == 8);
  REQUIRE(zb::base64::encoded_size(6) == 8);
  REQUIRE(zb::base64::encoded_size(7) == 12);
  REQUIRE(zb::base64::encoded_size(8) == 12);
}

TEST_CASE("Base64 : Encode") {
  constexpr const char* input = "john";
  constexpr std::string_view result = "am9obg==";

  {
    std::vector<uint8_t> encoded = zb::base64::encode<std::vector<uint8_t>>(input);
    REQUIRE(std::string_view((const char*)encoded.data(), encoded.size()) == result);

    std::vector<uint8_t> decoded = zb::base64::decode(encoded);
    REQUIRE(std::string_view((const char*)decoded.data(), decoded.size()) == input);
  }

  {
    std::string encoded = zb::base64::encode(input);
    REQUIRE(encoded == result);

    std::string decoded = zb::base64::decode<std::string>(std::string_view(encoded));
    REQUIRE(decoded == input);
  }

  //  {
  //    std::vector<char> encoded = uwm::base64::Encode<std::vector<char>>(input);
  //    REQUIRE(std::string_view(encoded.data(), encoded.size()) == result);
  //  }
  //
  //  {
  //    std::string encoded = uwm::base64::Encode(input);
  //    REQUIRE(encoded == result);
  //  }
  //
  //  {
  //    std::string encoded = uwm::base64::Encode<std::string>(input);
  //    REQUIRE(encoded == result);
  //  }
}
//
// TEST_CASE("Base64 : Decode", "[uwmbase]") {
//  constexpr const char* input = "bGlnaHQgd29yay4=";
//  constexpr std::string_view result = "light work.";
//
//  {
//    std::vector<uint8_t> decoded = uwm::base64::Decode(input);
//    REQUIRE(std::string_view((const char*)decoded.data(), decoded.size()) == result);
//  }
//
//  {
//    std::vector<char> decoded = uwm::base64::Decode<std::vector<char>>(input);
//    REQUIRE(std::string_view((const char*)decoded.data(), decoded.size()) == result);
//  }
//
//  {
//    std::string decoded = uwm::base64::Decode<std::string>(input);
//    REQUIRE(decoded == result);
//  }
//
//  {
//    std::string decoded = uwm::base64::Decode<std::string>(std::string_view(input));
//    REQUIRE(decoded == result);
//  }
//
//  {
//    std::string_view inputView(input);
//    std::string decoded = uwm::base64::Decode<std::string>(inputView.data(), inputView.size());
//    REQUIRE(decoded == result);
//  }
//
//  {
//    std::string_view inputView(input);
//    std::vector<uint8_t> vecInput;
//    vecInput.resize(inputView.size());
//    std::memcpy(vecInput.data(), inputView.data(), inputView.size());
//    std::string decoded = uwm::base64::Decode<std::string>(vecInput);
//    REQUIRE(decoded == result);
//  }
//}
//
// static constexpr std::array<std::string_view, 12> kInputStrings = {
//  "A", //
//  "Ab", //
//  "AbC", //
//  "AbCd", //
//  "AbCde", //
//  "AbCdef", //
//  "AbCdefg", //
//  "AbCdefgH", //
//  "AbCdefgHi", //
//  "ABCDXYZ abcdxyz 0123456789", //
//  "..!@#$%^&*()_+==", //
//  "1234567890-[];',./<>?:\"{}\\" //
//};
//
// TEST_CASE("Base64 : Encode Decode", "[uwmbase]") {
//  for (std::string_view input : kInputStrings) {
//    std::string encoded = uwm::base64::Encode(input);
//    REQUIRE(!encoded.empty());
//
//    std::vector<uint8_t> decoded = uwm::base64::Decode(encoded);
//    REQUIRE(!decoded.empty());
//    REQUIRE(decoded.size() == input.size());
//
//    std::string_view output((const char*)decoded.data(), decoded.size());
//    REQUIRE(output == input);
//  }
//}
//
// TEST_CASE("Base64 : Encode Decode float", "[uwmbase]") {
//
//  std::vector<float> input = { 1, 2, 3, 4 };
//  std::string encoded = uwm::base64::Encode((const uint8_t*)input.data(), input.size() * sizeof(float));
//  REQUIRE(!encoded.empty());
//
//  std::vector<uint8_t> decoded = uwm::base64::Decode(encoded);
//  REQUIRE(!decoded.empty());
//  REQUIRE(decoded.size() == input.size() * sizeof(float));
//
//  std::vector<float> output;
//  output.resize(input.size());
//  std::memcpy(output.data(), decoded.data(), decoded.size());
//
//  REQUIRE(output == input);
//}
//
// TEST_CASE("Base64 : Encode to Buffer", "[uwmbase]") {
//
//  std::vector<float> input = { 1, 2, 3, 4 };
//  size_t inputDataSize = input.size() * sizeof(float);
//
//  std::vector<uint8_t> encoded;
//  size_t encodedInputSize = uwm::base64::get_encoded_size(inputDataSize);
//  encoded.resize(encodedInputSize);
//
//  size_t encodedSize = uwm::base64::Encode((const uint8_t*)input.data(), inputDataSize,
//  (char*)encoded.data()); REQUIRE(encodedSize == encodedInputSize);
//
//  std::vector<uint8_t> decoded = uwm::base64::Decode(encoded);
//  REQUIRE(!decoded.empty());
//  REQUIRE(decoded.size() == inputDataSize);
//
//  std::vector<float> output;
//  output.resize(input.size());
//  std::memcpy(output.data(), decoded.data(), decoded.size());
//
//  REQUIRE(output == input);
//}

//
// #ifdef __APPLE__
// namespace base64_tests {
// extern std::string Base64PlatformEncode(const uint8_t* data, size_t size);
//} // namespace base64_tests.
//
// namespace {
// TEST_CASE("Base64 : Platform", "[uwmbase]") {
//  for (std::string_view input : kInputStrings) {
//    std::string encoded = uwm::base64::Encode(input);
//    std::string platform_encoded = base64_tests::Base64PlatformEncode((const uint8_t*)input.data(),
//    input.size()); REQUIRE(encoded == platform_encoded);
//  }
//}
//
// #if UNIT_TESTS_RUN_BASE64_BENCHMARKS
// TEST_CASE("Base64 : Benchmarks", "[uwmbase]") {
//  BENCHMARK_ADVANCED("Benchmark : Base64 - Encode")(Catch::Benchmark::Chronometer meter) {
//    meter.measure([] {
//      size_t count = 0;
//      for (std::string_view input : kInputStrings) {
//        std::string encoded = uwm::base64::Encode(input);
//        count += encoded.size();
//      }
//      return count;
//    });
//  };
//
//  BENCHMARK_ADVANCED("Benchmark : Base64 - Platform encode")(Catch::Benchmark::Chronometer meter) {
//    meter.measure([] {
//      size_t count = 0;
//      for (std::string_view input : kInputStrings) {
//        std::string platform_encoded = base64_tests::Base64PlatformEncode((const uint8_t*)input.data(),
//        input.size()); count += platform_encoded.size();
//      }
//      return count;
//    });
//  };
//}
// #endif // UNIT_TESTS_RUN_BASE64_BENCHMARKS
//

//
// #endif // __APPLE__
