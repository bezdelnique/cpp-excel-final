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

struct CellCacheStat {
 public:
  inline static size_t hit = 0;
  inline static size_t missed = 0;
  inline static size_t invalidate = 0;

  static void Reset() {
    hit = 0;
    missed = 0;
    invalidate = 0;
  }
};

class Cell : public CellInterface {
 private:
  class CellValue {
   public:
    virtual Value GetValue() = 0;
    virtual std::string GetText() = 0;
    virtual CellType GetType() = 0;
    virtual void InvalidateCache() {
    }
    virtual bool IsValid() {
      return true;
    }
    virtual std::vector<Position> GetReferencedCells() {
      throw std::logic_error("Not implemented"s);
    }
  };

  class CellValueEmpty : public CellValue {
    Value GetValue() override {
      throw std::logic_error("Access to empty cell"s);
    }

    std::string GetText() override {
      throw std::logic_error("Access to empty cell"s);
    }

    virtual CellType GetType() override {
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

    std::string GetText() override {
      return raw_value_;
    }

    CellType GetType() override {
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
          referenced_cells_(ValidateExpressionAndInitForwardList(raw_value)) {
    }

    std::vector<Position> ValidateExpressionAndInitForwardList(std::string expr) {
      // Попытка разобрать формулу
      try {
        std::unique_ptr<FormulaInterface> formula = ParseFormula(expr.substr(1));
        // Already sorted
        return formula->GetReferencedCells();
      } catch (...) {
        throw FormulaException("Unable to parse formula");
      }
    }

    Value GetValue() override {
      if (!IsValid()) {
        return FormulaError(FormulaError::Category::Ref);
      }

      if (cached_.has_value()) {
        ++CellCacheStat::hit;
      } else {
        ++CellCacheStat::missed;

        auto result = ParseFormula(raw_value_.substr(1))->Evaluate(sheet_);
        // Ошибки не кэшируем
        if (std::holds_alternative<FormulaError>(result)) {
          return std::get<FormulaError>(result);
        } else {
          cached_ = std::get<double>(result);
        }
      }

      return cached_.value();
    }

    std::string GetText() override {
      // Очищенная формула
      std::stringstream ss;
      ss << '=' << ParseFormula(raw_value_.substr(1))->GetExpression();
      return ss.str();
    }

    CellType GetType() override {
      return CellType::FORMULA;
    }

    bool IsValid() override {
      for (auto const &pos : referenced_cells_) {
        if (pos == Position::NONE) {
          return false;
        }
      }
      return true;
    }

    std::vector<Position> GetReferencedCells() override {
      return referenced_cells_;
    }

    void InvalidateCache() override {
      ++CellCacheStat::invalidate;
      cached_.reset();
    }

   private:
    SheetInterface &sheet_;
    std::vector<Position> referenced_cells_;
    std::optional<double> cached_;
  };

 public:
  Cell(SheetInterface &sheet);
  ~Cell();

  void Set(std::string text);
  // void Clear();

  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;

  void InvalidateCache() const;

  bool IsFormula() const;
  bool IsValid() const;

 private:
  SheetInterface &sheet_;
  std::unique_ptr<CellValue> value_holder_;
};

class PositionHasher {
 public:
  size_t operator()(const Position position) const;
};

std::ostream &operator<<(std::ostream &output, const CellInterface::Value &value);
