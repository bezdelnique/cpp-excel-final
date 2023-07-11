#include "common.h"

#include <cctype>
#include <sstream>
#include <cmath>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_DIGIT_COUNT = 8;
const int MAX_POS_LETTER_COUNT = 3;
const int MAX_ROWS_ZB = Position::MAX_ROWS - 1;
const int MAX_COLS_ZB = Position::MAX_COLS - 1;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
  return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
  if (col == rhs.col) {
    return row < rhs.row;
  }
  return col < rhs.col;
}

bool Position::IsValid() const {
  if (row < 0 || col < 0 || row > MAX_ROWS_ZB || col > MAX_COLS_ZB) {
    return false;
  }
  return true;
}

std::string Position::ToString() const {
  if (col > MAX_COLS_ZB || row > MAX_ROWS_ZB || col < 0 || row < 0) {
    return "";
  }

  // Определить разрядность
  // ZZ - (26*26) + 26
  static int zz = (LETTERS * LETTERS) + LETTERS;

  std::stringstream ss;
  // Чтобы математика складывалась
  //int col = col + 1;
  if (col > (zz - 1)) {
    int first = col / (LETTERS * LETTERS);
    int remains = col - ((LETTERS * LETTERS) * first);
    int second = remains / LETTERS;
    int third = remains - (LETTERS * second);
    ss << static_cast<char>('A' + first - 1)
       << static_cast<char>('A' + second - 1)
       << static_cast<char>('A' + third);
  } else if (col > (LETTERS - 1)) {
    size_t first = col / LETTERS;
    size_t second = col % LETTERS;
    ss << static_cast<char>('A' + first - 1)
       << static_cast<char>('A' + second);
  } else {
    ss << static_cast<char>('A' + col);
  }

  ss << row + 1;
  return ss.str();
}

Position Position::FromString(std::string_view str) {
  // MAX: XFD16384
  if (str.empty() || str.size() < 2 || str.size() > (MAX_POS_DIGIT_COUNT + MAX_POS_LETTER_COUNT)) {
    return Position::NONE;
  }

  bool digit = false;
  std::string col_str;
  std::string row_str;
  for (const char &ch : str) {
    if (!std::isdigit(ch) && !std::isupper(ch)) {
      return Position::NONE;
    }

    if (std::isdigit(ch)) {
      digit = true;
      row_str.push_back(ch);
    } else {
      col_str.push_back(ch);
    }

    if (digit && !std::isdigit(ch)) {
      return Position::NONE;
    }
  }

  if (col_str.size() > MAX_POS_LETTER_COUNT || row_str.size() > MAX_POS_DIGIT_COUNT) {
    return Position::NONE;
  }
  int row = std::stoi(row_str) - 1;

  if (row > MAX_ROWS_ZB) {
    return Position::NONE;
  }

  int col = 0;
  if (col_str.size() == 1) {
    col = col_str[0] - 'A';
  } else if (col_str.size() == 2) {
    col = (LETTERS * (col_str[0] - 'A' + 1)) + col_str[1] - 'A';
  } else {
    // MAX: XFD
    col = ((LETTERS * LETTERS) * (col_str[0] - 'A' + 1))
        + (LETTERS * (col_str[1] - 'A' + 1))
        + col_str[2] - 'A';
  }

  if (col > MAX_COLS_ZB) {
    return Position::NONE;
  }

  return {row, col};
}

bool Size::operator==(Size rhs) const {
  return rows == rhs.rows && cols == rhs.cols;
}
