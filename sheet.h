#pragma once

#include "cell.h"
#include "common.h"
#include <functional>
#include <unordered_map>
#include <map>

//using Row = std::vector<std::unique_ptr<Cell>>;

class Sheet : public SheetInterface {
 private:
  class BackwardListManager {
   public:
    void AddBackwardLink(Position to, Position from) {
//      auto it = backward_list_[from].find(to);
//      if (it != backward_list_[from].end()) {
//        throw std::logic_error("Backlink already exists"s);
//      }
      // Возможно добавление повторяющихся значений: =C3 + B2 / C3
      backward_list_[from].emplace(to);
    }

    void RemoveBackwardLink(Position to, Position from) {
      auto it = backward_list_[from].find(to);
      if (it == backward_list_[from].end()) {
        throw std::logic_error("Deleted backlink does not exists"s);
      }
      backward_list_[from].erase(it);
    }

    std::vector<Position> GetBackwardList(Position from) const {
      if (backward_list_.count(from) > 0) {
        std::vector vec(backward_list_.at(from).begin(), backward_list_.at(from).end());
        return vec;
      }
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
  //std::vector<Row> table_;
  std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> storage_;
  std::map<int, size_t> cols;
  std::map<int, size_t> rows;
  BackwardListManager backward_list_manager_;

  void afterClear(Position pos);
  void afterSet(Position pos);
  static void validatePosition(Position pos);

  bool CycleDetector(Position position, const CellInterface &cell);
  void UpdateBackwardLink(Position pos, const std::unique_ptr<Cell> &new_cell);
  void InvalidateCache(Position pos);
};
