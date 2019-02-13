#include <stdlib.h>
#include "entities.h"

int main() {
    Record record = create_record("De se +xwrw", "P. Pantelidis", 90);
    print_record(&record);
    return EXIT_SUCCESS;
}