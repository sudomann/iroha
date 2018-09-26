/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/common_objects/amount.hpp"

#include <algorithm>
#include <regex>

#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {
    Amount::Amount(const std::string &amount) : Amount(std::string(amount)) {}
    Amount::Amount(std::string &&amount)
        : amount_(std::move(amount)),
          precision_(0),
          multiprecision_repr_([this] {
            static const std::regex r(
                "[1-9][0-9]*(\\.([0-9]+))?|0\\.([0-9]*[1-9])");
            // 123.456 will have the following groups:
            //   [0] -> 123.456  [1] -> .456  [2] -> 456  [3] ->
            //
            // With leading zero, e.g 0.789 will have the following groups:
            //   [0] -> 0.789    [1] ->       [2] ->      [3] -> 789
            std::smatch match;
            if (std::regex_match(this->amount_, match, r)
                && match.size() == 4) {
              this->precision_ = std::max(match[2].length(), match[3].length());
              auto str = match[0].str();
              // remove dot if it exist
              auto pos = str.find('.');
              if (pos != std::string::npos) {
                str.erase(str.begin() + pos);
              }
              return boost::multiprecision::uint256_t(str);
            }
            return std::numeric_limits<boost::multiprecision::uint256_t>::min();
          }()) {}

    Amount::Amount(const Amount &o) : Amount(std::string(o.amount_)) {}

    Amount::Amount(Amount &&o) noexcept
        : Amount(std::move(const_cast<std::string &>(o.amount_))) {}

    const boost::multiprecision::uint256_t &Amount::intValue() const {
      return multiprecision_repr_;
    }

    types::PrecisionType Amount::precision() const {
      return precision_;
    }

    std::string Amount::toStringRepr() const {
      return amount_;
    }

    bool Amount::operator==(const ModelType &rhs) const {
      return amount_ == rhs.amount_;
    }

    std::string Amount::toString() const {
      return detail::PrettyStringBuilder()
          .init("Amount")
          .append("value", amount_)
          .finalize();
    }

    Amount *Amount::clone() const {
      return new Amount(*this);
    }
  }  // namespace interface
}  // namespace shared_model
