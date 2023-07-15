#pragma once

#include "cell.h"
#include "common.h"
#include <functional>
#include <unordered_map>
#include <map>

using Row = std::vector<std::unique_ptr<Cell>>;

class Sheet : public SheetInterface {
 private:
  class BackwardListManager {
    void AddBackwardLink(Position from, Position to) {
//      auto it = backward_list_.find(pos);
//      if (it != backward_list_.end()) {
//        throw std::logic_error("Удаление существуюей обратной ссылки"s);
//      }
//      backward_list_.emplace(pos);
    }

    void RemoveBackwardLink(Position from, Position to) {
//      auto it = backward_list_.find(pos);
//      if (it == backward_list_.end()) {
//        throw std::logic_error("Удаление несуществуюей обратной ссылки"s);
//      }
//      backward_list_.erase(it);
    }

    std::vector<Position> GetBackwardList(Position pos) const {
//      std::vector vec(backward_list_.at(pos).begin(), backward_list_.at(pos).end());
//      return vec;
      return {};
    }

   private:
    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> backward_list_;
  };

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
  BackwardListManager backward_list_manager_;

  void afterClear(Position pos);
  void afterSet(Position pos);
  void validatePosition(Position pos) const;

  bool CycleDetector(Position position, const CellInterface &cell);
  void UpdateBackwardLink(Position pos, const std::unique_ptr<Cell> &new_cell);
};
