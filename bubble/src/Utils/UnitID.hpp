#ifndef _TKET_UnitID_H_
#define _TKET_UnitID_H_

namespace tket {

enum class UnitType { Qubit };

class UnitID {
 public:
  UnitID() {}
};

/** Location holding a qubit */
class Qubit : public UnitID {
 public:
  Qubit() : UnitID() {}
};

}  // namespace tket

#endif
