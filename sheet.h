#pragma once

#include "cell.h"
#include "common.h"
#include <functional>
#include <unordered_map>
#include <map>

//class PositionHasher {
// public:
//  size_t operator()(const Position position) const;
//};

using Row = std::vector<std::unique_ptr<Cell>>;

class Sheet : public SheetInterface {
 public:
  ~Sheet();

  void SetCell(Position pos, std::string text) override;

  const CellInterface *GetCell(Position pos) const override;
  CellInterface *GetCell(Position pos) override;

  void ClearCell(Position pos) override;

  Size GetPrintableSize() const override;

  void PrintValues(std::ostream &output) const override;
  void PrintTexts(std::ostream &output) const override;

 private:
  std::vector<Row> table_;
  std::map<int, size_t> cols;
  std::map<int, size_t> rows;

  void afterClear(Position pos);
  void afterSet(Position pos);
  void validatePosition(Position pos) const;
};
