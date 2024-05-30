#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>

#define SIZE 20

struct DataItem {
    std::string data;
    char key;

    DataItem(char k, const std::string &d) : key(k), data(d) {}
};

DataItem* hashArray[SIZE];
DataItem* dummyItem = new DataItem(-1, "");
DataItem* item;

int hashCode(char key) {
    return key % SIZE;
}

DataItem* search(char key) {
    int hashIndex = hashCode(key);

    while (hashArray[hashIndex] != nullptr) {
        if (hashArray[hashIndex]->key == key)
            return hashArray[hashIndex];

        ++hashIndex;
        hashIndex %= SIZE;
    }

    assert(false && "ERROR: Not found in hashmap\n");
    return nullptr;
}

void insert(char key, const std::string &data) {
    DataItem* item = new DataItem(key, data);

    int hashIndex = hashCode(key);

    while (hashArray[hashIndex] != nullptr && hashArray[hashIndex]->key != -1) {
        ++hashIndex;
        hashIndex %= SIZE;
    }

    hashArray[hashIndex] = item;
}

DataItem* deleteItem(DataItem* item) {
    char key = item->key;

    int hashIndex = hashCode(key);

    while (hashArray[hashIndex] != nullptr) {
        if (hashArray[hashIndex]->key == key) {
            DataItem* temp = hashArray[hashIndex];

            hashArray[hashIndex] = dummyItem;
            return temp;
        }

        ++hashIndex;
        hashIndex %= SIZE;
    }

    return nullptr;
}

void display() {
    for (int i = 0; i < SIZE; ++i) {
        if (hashArray[i] != nullptr)
            std::cout << " (" << hashArray[i]->key << "," << hashArray[i]->data << ")";
        else
            std::cout << " ~~ ";
    }

    std::cout << std::endl;
}

int main() {
    insert('a', "apple");
    insert('b', "banana");
    insert('c', "cherry");
    insert('d', "date");

    display();

    item = search('b');
    if (item != nullptr) {
        std::cout << "Item found: (" << item->key << ", " << item->data << ")" << std::endl;
    } else {
        std::cout << "Item not found" << std::endl;
    }

    deleteItem(item);
    display();

    return 0;
}
