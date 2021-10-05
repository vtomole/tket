#ifndef _TKET_UnitID_H_
#define _TKET_UnitID_H_

#include <memory>
#include <vector>

namespace tket {

enum class UnitType { Qubit };

class UnitID {
 public:
  UnitID() : data_(std::make_shared<UnitData>()) {}

 protected:
  UnitID(const std::vector<unsigned> &index, UnitType type)
      : data_(std::make_shared<UnitData>(index, type)) {}

 private:
  struct UnitData {
    std::vector<unsigned> index_;
    UnitType type_;

    UnitData() : index_(), type_(UnitType::Qubit) {}
    UnitData(const std::vector<unsigned> &index,
        UnitType type)
        : index_(index), type_(type) {
    }
  };
  std::shared_ptr<UnitData> data_;
};

/** Location holding a qubit */
class Qubit : public UnitID {
 public:
  Qubit() : UnitID({}, UnitType::Qubit) {}
};

}  // namespace tket

#endif
