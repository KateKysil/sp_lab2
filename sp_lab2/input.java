// Example Java program

package demo;

import java.util.*;

public class Main {
    public static void main(String[] args) {
        int a = 42;
        double b = 3.14;
        long hex = 0x1A2B;
        float exp = 1.23e4f;
        char ch = 'A';
        String msg = "Hello, world!\n";

        // Arithmetic
        int sum = a + 10;
        int diff = a - 5;
        int mult = a * 2;
        double div = b / 2.0;

        // Logical
        boolean ok = (a > 0) && (b < 10.0);

        /* Multiline comment
           describing something important
        */
        if (ok) {
            System.out.println(msg);
        } else {
            System.out.println("Condition failed");
        }
    }
}
