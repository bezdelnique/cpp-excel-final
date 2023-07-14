#pragma once

#include "common.h"
#include "formula.h"
#include <optional>
#include <utility>

//enum CellType {
//  STRING,
//  FORMULA
//};

using namespace std::literals;

class Cell : public CellInterface {
 private:
  class Impl {
   public:
    virtual Value GetValue() = 0;
    virtual std::string GetText() = 0;
  };

  class EmptyImpl : public Impl {
    virtual Value GetValue() {
      throw std::logic_error("Access to empty cell"s);
    }

    virtual std::string GetText() {
      throw std::logic_error("Access to empty cell"s);
    }
  };

  class TextImpl : public Impl {
   public:
    explicit TextImpl(std::string raw_value) : raw_value_(std::move(raw_value)) {}

    Value GetValue() override {
      if (raw_value_[0] == ESCAPE_SIGN) {
        return raw_value_.substr(1);
      } else {
        return raw_value_;
      }
    }

    std::string GetText() {
      return raw_value_;
    }

   protected:
    std::string raw_value_;
  };

  class FormulaImpl : public TextImpl {
   public:
    explicit FormulaImpl(SheetInterface &sheet, const std::string &raw_value)
        : TextImpl(raw_value), sheet_(sheet) {

      formula_ = std::string_view(raw_value_.substr(1));
      //
    }
//   public:
//    explicit FormulaImpl(const std::string &raw_value) : Impl(raw_value) {
//      // Попытка разобрать формулу
//      try {
//        std::unique_ptr<FormulaInterface> formula = ParseFormula(raw_value.substr(1));
//        std::vector<Position> refs = formula->GetReferencedCells();
//        if (refs.size() > 1) {
//          for (auto const &ref : refs) {
//            auto coll = sheet_.GetCell(ref);
//          }
//        }
//      } catch (...) {
//        throw FormulaException("Unable to parse formula");
//      }

    Value GetValue() override {
      auto result = ParseFormula(std::string(formula_))->Evaluate(sheet_);
      if (std::holds_alternative<FormulaError>(result)) {
        return std::get<FormulaError>(result);
      } else {
        return std::get<double>(result);
      }
    }

    std::string GetText() override {
      // Очищенная формула
      std::stringstream ss;
      ss << '=' << ParseFormula(std::string(formula_))->GetExpression();
      return ss.str();
    }

   private:
    SheetInterface &sheet_;
    std::string_view formula_;
  };

 public:
  Cell(SheetInterface &sheet);
  ~Cell();

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;

  void InvalidateCache() const;

  void AddBackward(Position pos);
  void RemoveBackward(Position pos);

 private:
  //CellType cell_type_{CellType::STRING};
  //std::string value_;
  SheetInterface &sheet_;
  std::optional<double> cached_;
  // Ячейки на которые ссылаемся
  std::forward_list<Position> forward_list_;
  // Ячейки которые ссылаются на нас
  // Нужно обеспечить быстрый поиск для последующего удаления
  std::set<Position> backward_list_;
  //std::string raw_value_;
  std::unique_ptr<Impl> value_holder_;
};

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value);
