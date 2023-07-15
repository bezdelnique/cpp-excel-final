#pragma once

#include "common.h"
#include "formula.h"
#include <optional>
#include <utility>

enum CellType {
  EMPTY,
  STRING,
  FORMULA
};

using namespace std::literals;

class Cell : public CellInterface {
 private:
  class CellValue {
   public:
    virtual Value GetValue() = 0;
    virtual std::string GetText() = 0;
    virtual CellType GetType() = 0;
    virtual std::vector<Position> GetReferencedCells() {
      throw std::logic_error("Not implemented"s);
    }
  };

  class CellValueEmpty : public CellValue {
    virtual Value GetValue() {
      throw std::logic_error("Access to empty cell"s);
    }

    virtual std::string GetText() {
      throw std::logic_error("Access to empty cell"s);
    }

    virtual CellType GetType() {
      return CellType::EMPTY;
    }
  };

  class CellValueText : public CellValue {
   public:
    explicit CellValueText(std::string raw_value) : raw_value_(std::move(raw_value)) {}

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

    virtual CellType GetType() {
      return CellType::STRING;
    }

   protected:
    std::string raw_value_;
  };

  class CellValueFormula : public CellValueText {
   public:
    explicit CellValueFormula(SheetInterface &sheet, const std::string &raw_value)
        : CellValueText(raw_value),
          sheet_(sheet),
          forward_list_(ValidateExpressionAndInitForwardList(raw_value)) {
      //formula_ = std::string_view(raw_value_.substr(1));

    }

    std::vector<Position> ValidateExpressionAndInitForwardList(std::string expr) {
      // Попытка разобрать формулу
      try {
        std::unique_ptr<FormulaInterface> formula = ParseFormula(expr.substr(1));
        // Sorted
        return formula->GetReferencedCells();
      } catch (...) {
        throw FormulaException("Unable to parse formula");
      }
    }

    Value GetValue() override {
      //auto result = ParseFormula(std::string(formula_))->Evaluate(sheet_);
      auto result = ParseFormula(raw_value_.substr(1))->Evaluate(sheet_);
      if (std::holds_alternative<FormulaError>(result)) {
        return std::get<FormulaError>(result);
      } else {
        return std::get<double>(result);
      }
    }

    std::string GetText() override {
      // Очищенная формула
      std::stringstream ss;
      //ss << '=' << ParseFormula(std::string(formula_))->GetExpression();
      ss << '=' << ParseFormula(raw_value_.substr(1))->GetExpression();
      return ss.str();
    }

    virtual CellType GetType() {
      return CellType::FORMULA;
    }

    virtual std::vector<Position> GetReferencedCells() {
      return forward_list_;
    }

   private:
    SheetInterface &sheet_;
    //std::string_view formula_;
    std::vector<Position> forward_list_;
  };

 public:
  Cell(SheetInterface &sheet);
  ~Cell();

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;
  //std::vector<Position> GetForwardList() const ;
  std::vector<Position> GetBackwardList() const;

  void InvalidateCache() const;

  void AddBackward(Position pos);
  void RemoveBackward(Position pos);

  bool IsFormula() const;

 private:
  //Position position_;
  //CellType cell_type_{CellType::STRING};
  //std::string value_;
  SheetInterface &sheet_;
  std::optional<double> cached_;
  // Ячейки на которые ссылаемся
  //std::vector<Position> forward_list_;
  // Ячейки которые ссылаются на нас
  // Нужно обеспечить быстрый поиск для последующего удаления
  std::set<Position> backward_list_;
  //std::string raw_value_;
  std::unique_ptr<CellValue> value_holder_;

  //bool CycleDetector(std::forward_list<Position> forward_list);
};

class PositionHasher {
 public:
  size_t operator()(const Position position) const;
};

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value);
