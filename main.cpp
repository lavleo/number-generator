#include <iostream>

void generate(int startNumber, int amount) {
    for (int i = 0; i < amount; i++) {
        std::cout << "C" <<  startNumber + i << std::endl;
    }
}

int main() {
    int startNumber, amount;
    char choice = 'y';
    
    std::cout << "Enter a number: ";
    std::cin >> startNumber;

    std::cout << "Enter the amount of numbers to generate: ";
    std::cin >> amount;

    std::cout << "\nGenerated numbers:" << std::endl;
    generate(startNumber, amount);

    while (choice == 'y' || choice == 'Y') {
        std::cout << "Do you want to generate more numbers? (y/n): ";
        std::cin >> choice;
        generate(startNumber+amount, amount);
        std::cout << std::endl;
    }
    return 0;
}
