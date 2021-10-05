#ifndef _TKET_UnitID_H_
#define _TKET_UnitID_H_

namespace tket {

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
