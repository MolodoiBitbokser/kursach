#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
using namespace std;

vector<vector<vector<int>>> read_matrices_file(string filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: could not open file \"" << filename << "\"" << endl;
        exit(1);
    }

    vector<vector<vector<int>>> matrices;
    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue; // skip empty lines
        }
        stringstream ss(line);
        double subgroup;
        char colon;
        ss >> subgroup >> colon;
        matrices.resize(subgroup + 1);
        string matrix_str;
        getline(ss, matrix_str, ';');
        if (subgroup < 0 || subgroup >= matrices.size()) {
            cerr << "Error: subgroup index out of bounds: " << subgroup << endl;
            exit(1);
        }
        matrices[subgroup].push_back(parse_matrix_string(matrix_str));
    }
    file.close();

    return matrices;
}

vector<int> parse_row_string(string row_str) {
    vector<int> row;
    stringstream ss(row_str);
    char open_bracket, close_bracket, comma;
    ss >> open_bracket;
    int value;
    while (ss >> value) {
        row.push_back(value);
        ss >> comma;
    }
    ss >> close_bracket;
    return row;
}

vector<vector<int>> parse_matrix_string(string matrix_str) {
    vector<vector<int>> matrix;
    stringstream ss(matrix_str);
    string row_str;
    char open_brace, close_brace, semicolon;
    ss >> open_brace;
    while (getline(ss, row_str, ';')) {
        matrix.push_back(parse_row_string(row_str));
        ss >> semicolon;
    }
    ss >> close_brace;
    return matrix;
}

vector<vector<int>> multiply_matrices(vector<vector<int>> a, vector<vector<int>> b) {
    int rows_a = a.size();
    int cols_a = a[0].size();
    int cols_b = b[0].size();
    vector<vector<int>> c(rows_a, vector<int>(cols_b, 0));
    for (int i = 0; i < rows_a; i++) {
        for (int j = 0; j < cols_b; j++) {
            for (int k = 0; k < cols_a; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return c;
}

void write_matrix_file(string filename, int subgroup, vector<vector<int>> matrix) {
    ofstream file(filename, ios::app);
    if (!file) {
        cerr << "Error: could not open file \"" << filename << "\"" << endl;
        exit(1);
    }

    file << subgroup << " : {" << endl;
    for (vector<int> row : matrix) {
        file << "\t[ ";
        for (int x : row) {
            file << x << ", ";
        }
        file << "]" << endl;
    }
    file << "};" << endl;

    file.close();
}

int main() {
    // Read matrix file and parse into vector of matrix vectors
    vector<vector<vector<int>>> matrices = read_matrices_file("matrices.txt");

    // Compute product matrix for every pair of matrices
    vector<vector<vector<int>>> products;
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            vector<vector<int>> product = multiply_matrices(matrices[i][0], matrices[j][0]);
            products.push_back({ product, {i, j} });
        }
    }

    // Record duplicates in file
    ofstream file("knownDuplicates.txt");
    for (int i = 0; i < products.size(); i++) {
        vector<vector<int>> a = products[i][0];
        vector<pair<int, int>> a_subgroups = products[i][1];
        for (int j = i + 1; j < products.size(); j++) {
            vector<vector<int>> b = products[j][0];
            vector<pair<int, int>> b_subgroups = products[j][1];
            if (a == b) {
                file << a_subgroups[0].first << a_subgroups[0].second << " = "
                    << b_subgroups[0].first << b_subgroups[0].second << endl;
            }
        }
    }
    file.close();

    // Analyze duplicates for patterns and equations between subgroups
    ofstream result_file("resultAnalysis.txt");
    ifstream input_file("knownDuplicates.txt");
    string line;
    while (getline(input_file, line)) {
        stringstream ss(line);
        string lhs_str, rhs_str;
        getline(ss, lhs_str, '=');
        getline(ss, rhs_str);
        int a = stoi(lhs_str.substr(0, 1));
        int b = stoi(lhs_str.substr(1, 1));
        int c = stoi(rhs_str.substr(0, 1));
        int d = stoi(rhs_str.substr(1, 1));
        vector<vector<int>> lhs_product = multiply_matrices(matrices[a][0], matrices[b][0]);
        vector<vector<int>> rhs_product = multiply_matrices(matrices[c][0], matrices[d][0]);
        if (lhs_product == rhs_product) {
            result_file << "Found equation: U_" << a << " V_" << b << " U_" << c << " V_" << d << "=E" << endl;
        }
    }
    input_file.close();
    result_file.close();

    return 0;
}