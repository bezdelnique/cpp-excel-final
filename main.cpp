#include "common.h"
#include "cell.h"
#include "sheet.h"
#include "formula.h"
#include "test_runner_p.h"
#include <vector>
#include <memory>
#include <cassert>
#include <sstream>

using namespace std::literals;
using namespace std;

inline std::ostream &operator<<(std::ostream &output, Position pos) {
  return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator "" _pos(const char *str, std::size_t) {
  return Position::FromString(str);
}

inline std::ostream &operator<<(std::ostream &output, Size size) {
  return output << "(" << size.rows << ", " << size.cols << ")";
}

namespace {

void TestEmpty() {
  auto sheet = CreateSheet();
  ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{0, 0}));
}

void TestInvalidPosition() {
  auto sheet = CreateSheet();
  try {
    sheet->SetCell(Position{-1, 0}, "");
  } catch (const InvalidPositionException &) {
  }
  try {
    sheet->GetCell(Position{0, -2});
  } catch (const InvalidPositionException &) {
  }

  try {
    sheet->ClearCell(Position{Position::MAX_ROWS, 0});
  } catch (const InvalidPositionException &) {
  }

}

void TestSetCellPlainText() {
  auto sheet = CreateSheet();

  auto checkCell = [&](Position pos, std::string text) {
    sheet->SetCell(pos, text);
    CellInterface *cell = sheet->GetCell(pos);
    ASSERT(cell != nullptr);
    ASSERT_EQUAL(cell->GetText(), text);
    ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text);
  };

  checkCell("A1"_pos, "Hello");
  checkCell("A1"_pos, "World");
  checkCell("B2"_pos, "Purr");
  checkCell("A3"_pos, "Meow");

  const SheetInterface &constSheet = *sheet;
  ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr");

  sheet->SetCell("A3"_pos, "'=escaped");
  CellInterface *cell = sheet->GetCell("A3"_pos);
  ASSERT_EQUAL(cell->GetText(), "'=escaped");
  ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped");
}

void TestClearCell() {
  auto sheet = CreateSheet();

  sheet->SetCell("C2"_pos, "Me gusta");
  sheet->ClearCell("C2"_pos);
  ASSERT(sheet->GetCell("C2"_pos) == nullptr);

  sheet->ClearCell("A1"_pos);
  sheet->ClearCell("J10"_pos);
}
void TestPrint() {
  auto sheet = CreateSheet();
  sheet->SetCell("A2"_pos, "meow");
  sheet->SetCell("B2"_pos, "=1+2");
  sheet->SetCell("A1"_pos, "=1/0");

  ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 2}));

  std::ostringstream texts;
  sheet->PrintTexts(texts);
  //auto _ = texts.str();
  ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n");

  std::ostringstream values;
  sheet->PrintValues(values);
  ASSERT_EQUAL(values.str(), "#DIV/0!\t\nmeow\t3\n");

  sheet->ClearCell("B2"_pos);
  ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{2, 1}));
}

void PrintSheet(const std::unique_ptr<SheetInterface> &sheet, std::ostream &out) {
  out << sheet->GetPrintableSize() << std::endl;
  sheet->PrintTexts(out);
  out << std::endl;
  sheet->PrintValues(out);
  out << std::endl;
}

void TestExample() {
  auto sheet = CreateSheet();
  sheet->SetCell("A1"_pos, "=(1+2)*3");
  sheet->SetCell("B1"_pos, "=1+(2*3)");

  sheet->SetCell("A2"_pos, "some");
  sheet->SetCell("B2"_pos, "text");
  sheet->SetCell("C2"_pos, "here");

  sheet->SetCell("C3"_pos, "'and'");
  sheet->SetCell("D3"_pos, "'here");

  sheet->SetCell("B5"_pos, "=1/0");

  ostringstream out;
  PrintSheet(sheet, out);

  ostringstream etalon{
      "(5, 4)\n"s
          "=(1+2)*3\t=1+2*3\t\t\n"s
              "some\ttext\there\t\n"s
                  "\t\t'and'\t'here\n"
                  "\t\t\t\n"s
                      "\t=1/0\t\t\n"s
                          "\n"s
                              "9\t7\t\t\n"s
                                  "some\ttext\there\t\n"s
                                      "\t\tand'\there\n"
                                      "\t\t\t\n"s
                                          "\t#DIV/0!\t\t\n"s
                                              "\n"s
  };
  assert(out.str() == etalon.str());
  cerr << "TestExample OK"s << endl;
}

//void PrintSheet(const std::unique_ptr<SheetInterface> &sheet) {
//  std::cout << sheet->GetPrintableSize() << std::endl;
//  sheet->PrintTexts(std::cout);
//  std::cout << std::endl;
//  sheet->PrintValues(std::cout);
//  std::cout << std::endl;
//}

void PrintSheet(const std::unique_ptr<SheetInterface> &sheet, std::stringstream &ss) {
  ss << sheet->GetPrintableSize() << std::endl;
  sheet->PrintTexts(ss);
  ss << std::endl;
  sheet->PrintValues(ss);
  ss << std::endl;
}

void TestClearCells5x5() {
  {
    auto sheet = CreateSheet();
    for (int i = 0; i <= 5; ++i) {
      sheet->SetCell(Position{i, i}, std::to_string(i));
    }

    sheet->ClearCell(Position{3, 3});

    std::stringstream ss;
    for (int i = 5; i >= 0; --i) {
      sheet->ClearCell(Position{i, i});
      //PrintSheet(sheet);
      PrintSheet(sheet, ss);
    }
  }

}


// == TestSimple ==


void TestSimpleCell() {
  auto sheet = CreateSheet();

  // Set / reset
  {
    Cell cell{*sheet};
    cell.Set("text"s);
    assert(std::get<std::string>(cell.GetValue()) == "text"s);

    cell.Set("=1+1"s);
    assert(std::get<double>(cell.GetValue()) == 2);
  }

  cerr << "TestSimpleCell OK"s << endl;
}

void TestSimpleTableCell() {
  auto sheet = CreateSheet();
  sheet->SetCell("A1"_pos, "=1"s);
  sheet->SetCell("A2"_pos, "=1"s);

  //auto formula = ParseFormula("3+1");
  auto formula = ParseFormula("A1+A2");
  auto result = formula->Evaluate(*sheet);
  //auto refs = formula->GetReferencedCells();
  //std::cout << std::get<double>(result) << std::endl;
  assert(std::get<double>(result) == 2);

  cerr << "TestSimpleTableCell OK"s << endl;
}

void TestSimpleSearchCycles() {
  // Negative
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=3"s);
    sheet->SetCell("A2"_pos, "=A1+1"s);
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 4);
  }

  // Negative
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=3"s);
    sheet->SetCell("A2"_pos, "=A1+1"s);
    sheet->SetCell("A3"_pos, "=A1+A2"s);
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 4);
  }

  // Positive: self
  {
    auto sheet = CreateSheet();
    try {
      sheet->SetCell("A1"_pos, "=A1"s);
      assert(false);
    } catch (CircularDependencyException &) {
      assert(true);
    }
    catch (...) {
      assert(false);
    }
  }

  // Positive: Set
  {
    auto sheet = CreateSheet();
    try {
      sheet->SetCell("A1"_pos, "=3"s);
      sheet->SetCell("A3"_pos, "=C4+1"s);
      sheet->SetCell("C4"_pos, "=A2-1"s);
      sheet->SetCell("A2"_pos, "=A1+A3"s);
      assert(false);
    } catch (CircularDependencyException &) {
      assert(true);
    }
    catch (...) {
      assert(false);
    }
  }

  // Positive: Set
  {
    auto sheet = CreateSheet();
    try {
      sheet->SetCell("C1"_pos, "3");
      sheet->SetCell("A1"_pos, "=C1+A1");
      assert(false);
    } catch (CircularDependencyException &) {
      assert(true);
    }
    catch (...) {
      assert(false);
    }
  }

  // Positive: Set again
  {
    auto sheet = CreateSheet();
    try {
      sheet->SetCell("A1"_pos, "=3"s);
      sheet->SetCell("A2"_pos, "=A1+1"s);
      sheet->SetCell("A3"_pos, "=A1+A2"s);
      sheet->SetCell("A1"_pos, "=A3"s);
      assert(false);
    } catch (CircularDependencyException &) {
      assert(true);
    }
    catch (...) {
      assert(false);
    }
  }

  cerr << "TestSimpleSearchCycles OK"s << endl;
}

void TestSimpleCacheInvalidation() {
  // missed / hit
  {
    auto sheet = CreateSheet();
    CellCacheStat::Reset();
    sheet->SetCell("A1"_pos, "3"s);
    sheet->SetCell("A2"_pos, "=A1+1"s);
    assert(CellCacheStat::invalidate == 0);
    assert(CellCacheStat::hit == 0);
    assert(CellCacheStat::missed == 0);

    CellCacheStat::Reset();
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 4);
    assert(CellCacheStat::invalidate == 0);
    assert(CellCacheStat::hit == 0);
    assert(CellCacheStat::missed == 1);

    CellCacheStat::Reset();
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 4);
    assert(CellCacheStat::invalidate == 0);
    assert(CellCacheStat::hit == 1);
    assert(CellCacheStat::missed == 0);
  }


  // invalidate
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "3"s);
    sheet->SetCell("A2"_pos, "=A1+1"s); // 4
    sheet->SetCell("C4"_pos, "=A2+1"s); // 5

    assert(std::get<double>(sheet->GetCell("C4"_pos)->GetValue()) == 5);
    assert(CellCacheStat::invalidate == 0);

    CellCacheStat::Reset();
    sheet->SetCell("A1"_pos, "4"s);
    assert(CellCacheStat::invalidate == 2);
    assert(std::get<double>(sheet->GetCell("C4"_pos)->GetValue()) == 6);
  }

}

void TestClearEmptyCell() {
  {
    auto sheet = CreateSheet();
    for (int i = 0; i < 5; ++i) {
      try {
        sheet->ClearCell(Position{i, i});
      } catch (...) {
        assert(false);
      }
      assert(sheet->GetCell(Position{i, i}) == nullptr);
    }
    cerr << "TestClearEmptyCell OK"s << endl;
  }

}

void TestSimpleLinkToEmptyCell() {
  // Если формула содержит индекс пустой ячейки, предполагаем, что значение пустой ячейки — 0.
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A2"_pos, "=A1+2"s);
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 2);
  }

  {
    auto sheet = CreateSheet();
    sheet->SetCell("A2"_pos, "=2/A1"s);
    assert(std::get<FormulaError>(sheet->GetCell("A2"_pos)->GetValue()).GetCategory() == FormulaError::Category::Div0);
  }

  {
    auto sheet = CreateSheet();
    sheet->SetCell("A2"_pos, "=A1/2"s);
    assert(std::get<double>(sheet->GetCell("A2"_pos)->GetValue()) == 0);
  }

  cerr << "TestSimpleLinkToEmptyCell OK"s << endl;
}

void TestSimpleLinkToTextCell() {
  // Если ячейку, чей индекс входит в формулу, нельзя проинтерпретировать как число, возникает ошибка нового типа:
  // FormulaError — нет значения #VALUE! В следующем примере в ячейке А2 находится текст, поэтому вычисление формулы в ячейке С2 (=А3/А2) вернёт эту ошибку.
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "hello"s);
    sheet->SetCell("A2"_pos, "=A1+2"s);
    assert(std::get<FormulaError>(sheet->GetCell("A2"_pos)->GetValue()).GetCategory() == FormulaError::Category::Value);
  }

  cerr << "TestSimpleLinkToTextCell OK"s << endl;
}

void TestSimpleLinkOutOfBound() {
  // Формула может содержать ссылку на ячейку, которая выходит за границы возможного размера таблицы,
  // например C2 (=A1234567+ZZZZ1). Такая формула может быть создана, но не может быть вычислена, поэтому
  // её вычисление вернёт ошибку #REF!
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=A1234567+ZZZZ1"s);
    assert(std::get<FormulaError>(sheet->GetCell("A1"_pos)->GetValue()).GetCategory() == FormulaError::Category::Ref);
  }

  cerr << "TestSimpleLinkOutOfBound OK"s << endl;
}

void TestSimpleErrorPropagation() {
  // Ref
//  {
//    auto sheet = CreateSheet();
//    sheet->SetCell("A1"_pos, "=A1234567+ZZZZ1"s);
//    sheet->SetCell("A2"_pos, "=A1"s);
//    assert(std::get<FormulaError>(sheet->GetCell("A2"_pos)->GetValue()).GetCategory() == FormulaError::Category::Ref);
//  }

  // Div
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=1/0"s);
    sheet->SetCell("A2"_pos, "=A1"s);
    assert(std::get<FormulaError>(sheet->GetCell("A2"_pos)->GetValue()).GetCategory() == FormulaError::Category::Div0);
  }

  // Value
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "hello"s);
    sheet->SetCell("A2"_pos, "=A1+2"s);
    sheet->SetCell("A3"_pos, "=A2"s);
    assert(std::get<FormulaError>(sheet->GetCell("A3"_pos)->GetValue()).GetCategory() == FormulaError::Category::Value);
  }

  cerr << "TestSimpleErrorPropagation OK"s << endl;
}

void TestSimpleErrorText() {
  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "=1/0"s);

    std::stringstream ss;
    ss << sheet->GetCell("A1"_pos)->GetValue();
    assert(ss.str() == "#DIV/0!"s);
  }

  {
    auto sheet = CreateSheet();
    sheet->SetCell("A1"_pos, "hello"s);
    sheet->SetCell("A2"_pos, "=A1"s);

    std::stringstream ss;
    ss << sheet->GetCell("A2"_pos)->GetValue();
    assert(ss.str() == "#VALUE!"s);
  }

//  {
//    auto sheet = CreateSheet();
//    sheet->SetCell("A1"_pos, "=A1234567+ZZZZ1"s);
//
//    std::stringstream ss;
//    ss << sheet->GetCell("A1"_pos)->GetValue();
//    assert(ss.str() == "#REF!"s);
//  }

}

}  // namespace


int main() {
//  TestRunner tr;
//  RUN_TEST(tr, TestEmpty);
//  RUN_TEST(tr, TestInvalidPosition);
//  RUN_TEST(tr, TestSetCellPlainText);
//  RUN_TEST(tr, TestClearCell);
//  RUN_TEST(tr, TestPrint);
//  TestExample();
//  TestClearEmptyCell();
//  TestClearCells5x5();
//
//  TestSimpleCell();
//  TestSimpleTableCell();
  TestSimpleSearchCycles();
  TestSimpleCacheInvalidation();

  TestSimpleLinkToEmptyCell();
  TestSimpleLinkToTextCell();
  //TestSimpleLinkOutOfBound();
  TestSimpleErrorPropagation();
  TestSimpleErrorText();

  return 0;
}
