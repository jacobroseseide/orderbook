#include "orderbook.hpp"

int main() {
    // create the order book
    OrderBook book;

    // creates buy orders to add to the order book
    Order o1 {1, Side::buy, OrderType::limit, 10.50, 200};
    Order o2 {2, Side::buy, OrderType::limit, 11.01, 110};
    Order o3 {3, Side::buy, OrderType::limit, 10.75, 300};
    Order o4 {4, Side::buy, OrderType::limit, 10.50, 100};

    // creates sell orders to add to the order book
    Order o5 {5, Side::sell, OrderType::limit, 11.00, 250};
    Order o6 {6, Side::sell, OrderType::limit, 10.90, 175};
    Order o7 {7, Side::sell, OrderType::limit, 11.25, 400};
    Order o8 {8, Side::sell, OrderType::limit, 10.95, 200};

    // adds to buy or sell book depending on Side
    book.addOrder(o1);
    book.addOrder(o2);
    book.addOrder(o3);
    book.addOrder(o4);
    book.addOrder(o5);
    book.addOrder(o6);
    book.addOrder(o7);
    book.addOrder(o8);


    book.printBook();

    return 0;
}



// exchange sends struct:
// price, id, quantity, side

// add method param: struct

// delete method param: id

// modify method param: id, new quantity

// two classes buybook and sell book

// acting as the exchange, not matching

// price level class

// price levels are made up of orders --> vector of orders
// order book is made up of price levels
