#ifndef _TKET_UnitID_H_
#define _TKET_UnitID_H_

#include <memory>

namespace tket {

enum class UnitType { Qubit };

class UnitID {
 public:
  UnitID() : data_(std::make_shared<UnitData>()) {}

 protected:
  UnitID(UnitType type) : data_(std::make_shared<UnitData>(type)) {}

 private:
  struct UnitData {
    UnitType type_;

    UnitData() : type_(UnitType::Qubit) {}
    UnitData(UnitType type) : type_(type) {}
  };
  std::shared_ptr<UnitData> data_;
};

/** Location holding a qubit */
class Qubit : public UnitID {
 public:
  Qubit() : UnitID(UnitType::Qubit) {}
};

}  // namespace tket

#endif
