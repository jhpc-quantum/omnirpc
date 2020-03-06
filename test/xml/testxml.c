#include <xml_node.h>

int
main(void)
{
    char *filename  = "testxml.xml";
    xml_node t;
    FILE *fp;

    fp = fopen(filename, "r");
    t = xml_read_document(fp);
    xml_node_print(t, stdout);

    return 0;
}

