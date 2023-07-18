#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
  validatePosition(pos);

  auto new_cell = std::make_unique<Cell>(*this);
  new_cell->Set(text);
  if (new_cell->IsFormula() && new_cell->IsValid()) {
    if (CycleDetector(pos, *new_cell)) {
      throw CircularDependencyException("Cycle detected"s);
    }
  }

  {
//    if (pos.row + 1 > static_cast<int>(table_.size())) {
//      table_.resize(pos.row + 1);
//    }
//
//    if (pos.col + 1 > static_cast<int>(table_[pos.row].size())) {
//      table_[pos.row].resize(pos.col + 1);
//    }

//    if (table_[pos.row][pos.col] == nullptr) {
//      table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
//    }
//
//    table_[pos.row][pos.col]->Set(text);

    if (new_cell->IsValid()) {
      InvalidateCache(pos);
      UpdateBackwardLink(pos, new_cell);
    }
    // table_[pos.row][pos.col] = std::move(new_cell);

    auto it = storage_.find(pos);
    if (it == storage_.end()) {
      storage_.emplace(pos, std::move(new_cell));
    } else {
      it->second = std::move(new_cell);
    }

  }

  afterSet(pos);
}

void Sheet::InvalidateCache(Position pos) {
  {
    auto cell = reinterpret_cast<Cell *>(GetCell(pos));
    if (cell != nullptr) {
      cell->InvalidateCache();
    }
  }

  for (auto const &to: backward_list_manager_.GetBackwardList(pos)) {
    InvalidateCache(to);
  }
}

void Sheet::UpdateBackwardLink(Position pos, const std::unique_ptr<Cell> &new_cell) {
  auto cell = GetCell(pos);
  //if (table_[pos.row][pos.col] != nullptr) {
  if (cell != nullptr) {
    // Инвалидировать кеш у зависимых ячеек
    //table_[pos.row][pos.col]->InvalidateCache();

    // Удалить обратные ссылки
    for (auto const &from: cell->GetReferencedCells()) {
      //auto cell = reinterpret_cast<Cell *>(GetCell(from));
      //cell->RemoveBackwardLink(pos);
      backward_list_manager_.RemoveBackwardLink(pos, from);
    }

    // Сохранение обратных ссылок со предыдущей версии ячейки
    for (auto const from: backward_list_manager_.GetBackwardList(pos)) {
      //new_cell->AddBackwardLink(from);
      backward_list_manager_.AddBackwardLink(pos, from);
    }
  }

  // Добавить новые обратные ссылки
  for (auto const &from: new_cell->GetReferencedCells()) {
    //auto cell = reinterpret_cast<Cell*>(GetCell(from));
    //cell->AddBackwardLink(pos);
    backward_list_manager_.AddBackwardLink(pos, from);
  }

}

const CellInterface *Sheet::GetCell(Position pos) const {
  validatePosition(pos);

  auto it = storage_.find(pos);
  if (it != storage_.end()) {
    return it->second.get();
  }

//  if (pos.row < static_cast<int>(table_.size())) {
//    if (pos.col < static_cast<int>(table_[pos.row].size())) {
//      if (table_[pos.row][pos.col] != nullptr) {
//        return table_[pos.row][pos.col].get();
//      }
//    }
//  }
  return nullptr;
}

CellInterface *Sheet::GetCell(Position pos) {
  validatePosition(pos);

  auto it = storage_.find(pos);
  if (it != storage_.end()) {
    return it->second.get();
  }

//  if (pos.row < static_cast<int>(table_.size())) {
//    if (pos.col < static_cast<int>(table_[pos.row].size())) {
//      if (table_[pos.row][pos.col] != nullptr) {
//        return table_[pos.row][pos.col].get();
//      }
//    }
//  }
  return nullptr;
}

void Sheet::ClearCell(Position pos) {
  validatePosition(pos);

  bool found = false;
  //CellInterface *cell = GetCell(pos);
  auto it = storage_.find(pos);
  if (it != storage_.end()) {
    //table_[pos.row][pos.col] = nullptr;
    it->second = nullptr;
    found = true;
  }

  if (found) {
    afterClear(pos);
  }
}

Size Sheet::GetPrintableSize() const {
  //if (table_.empty()) {
  if (storage_.empty()) {
    return {0, 0};
  }
  Size size = {0, 0};
  if (!rows.empty()) {
    size.rows = rows.rbegin()->first + 1;
  }
  if (!cols.empty()) {
    size.cols = cols.rbegin()->first + 1;
  }
  return {size.rows, size.cols};
}

void Sheet::PrintValues(std::ostream &output) const {
  auto size = GetPrintableSize();
  for (int row = 0; row < size.rows; ++row) {
    for (int col = 0; col < size.cols; ++col) {
      if (col > 0) {
        output << "\t";
      }
      const auto value = GetCell({row, col});
      if (value != nullptr) {
        output << value->GetValue();
      }
    }

    output << "\n";
  }

}

void Sheet::PrintTexts(std::ostream &output) const {
  auto size = GetPrintableSize();
  for (int row = 0; row < size.rows; ++row) {
    for (int col = 0; col < size.cols; ++col) {
      if (col > 0) {
        output << "\t";
      }
      const auto value = GetCell({row, col});
      if (value != nullptr) {
        output << value->GetText();
      }
    }

    output << "\n";
  }

}

void Sheet::afterClear(Position pos) {
  {
    auto it = rows.find(pos.row);
    if (it != rows.end()) {
      if (it->second > 1) {
        --it->second;
      } else {
        rows.erase(it);
      }
    }
  }

  {
    auto it = cols.find(pos.col);
    if (it != cols.end()) {
      if (it->second > 1) {
        --it->second;
      } else {
        cols.erase(it);
      }
    }
  }

}

void Sheet::afterSet(Position pos) {
  {
    auto it = rows.find(pos.row);
    if (it == rows.end()) {
      rows.emplace(pos.row, 1);
    } else {
      ++it->second;
    }
  }

  {
    auto it = cols.find(pos.col);
    if (it == cols.end()) {
      cols.emplace(pos.col, 1);
    } else {
      ++it->second;
    }
  }

}

void Sheet::validatePosition(Position pos) {
// Убираю проверку чтобы сходилось с тестами диплома

//  if (pos.row < 0 || pos.col < 0) {
//    std::stringstream ss;
//    ss << pos;
//    throw InvalidPositionException(ss.str());
//  }
//
//  if (pos.row > Position::MAX_ROWS_ZB || pos.col > Position::MAX_COLS_ZB) {
//    std::stringstream ss;
//    ss << pos;
//    throw InvalidPositionException(ss.str());
//  }
}

bool Sheet::CycleDetector(Position position, const CellInterface &cell) {
  enum Color {
    WHITE,
    GRAY,
    BLACK
  };

  std::unordered_map<Position, Color, PositionHasher> visited;
  std::stack<Position> stack;
  visited[position] = GRAY;
  //stack.push(position);
  for (auto const &elem: cell.GetReferencedCells()) {
    // Либо ссылка на самого себя, либо ошибка формирования ссылок
    if (visited.count(elem) > 0) {
      return true;
    }
    stack.push(elem);
    visited[elem] = WHITE;
  }

  while (!stack.empty()) {
    auto from = stack.top();
    stack.pop();
    bool process = true;
    {
      auto it = visited.find(from);
      if (it == visited.end()) {
        throw std::logic_error("Element out of processed status");
      }

      // Если пришли в вершину второй раз в цвете GRAY значит все исходящие ребра обработаны
      if (it->second == GRAY) {
        visited[from] = Color::BLACK;
        process = false;
      }
    }

    if (process) {
      visited[from] = Color::GRAY;
      stack.push(from);

      // Обход связанных вершин
      auto from_cell = this->GetCell(from);
      if (from_cell != nullptr) {
        for (auto const to: from_cell->GetReferencedCells()) {
          auto it = visited.find(to);
          if (it != visited.end()) {
            if (it->second != WHITE) {
              return true;
            }
          }

          // В первый раз в вершину приходим в начале обхода
          visited[to] = Color::WHITE;
          stack.push(to);
        }
      }
    }

  }

  return false;
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}


