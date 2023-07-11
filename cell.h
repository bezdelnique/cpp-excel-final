#pragma once

#include "common.h"
#include "formula.h"

enum CellType {
  STRING,
  FORMULA
};

using namespace std::literals;

class Cell : public CellInterface {
 public:
  Cell();
  ~Cell();

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;

 private:
  CellType cell_type_{CellType::STRING};
  std::string value_;
  //std::string raw_value_;

//можете воспользоваться нашей подсказкой, но это необязательно.
/*    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
*/
};

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value);
