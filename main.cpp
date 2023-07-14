#include "common.h"
#include "cell.h"
#include "sheet.h"
#include "formula.h"
#include "test_runner_p.h"
#include <vector>
#include <memory>
#include <cassert>

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

void PrintSheet(const std::unique_ptr<SheetInterface> &sheet) {
  std::cout << sheet->GetPrintableSize() << std::endl;
  sheet->PrintTexts(std::cout);
  std::cout << std::endl;
  sheet->PrintValues(std::cout);
  std::cout << std::endl;
}

void TestClearCells5x5() {
  {
    auto sheet = CreateSheet();
    for (int i = 0; i <= 5; ++i) {
      sheet->SetCell(Position{i, i}, std::to_string(i));
    }

    sheet->ClearCell(Position{3, 3});

    for (int i = 5; i >= 0; --i) {
      sheet->ClearCell(Position{i, i});
      PrintSheet(sheet);
    }
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

  auto sheet = CreateSheet();
  sheet->SetCell("A1"_pos, "=1"s);
  sheet->SetCell("A2"_pos, "=1"s);

  //auto formula = ParseFormula("3+1");
  auto formula = ParseFormula("A1+A2");
  auto result = formula->Evaluate(*sheet);
  auto refs = formula->GetReferencedCells();
  std::cout << std::get<double>(result) << std::endl;

  return 0;
}
