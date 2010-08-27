#include <stdio.h>
#include <gcalendar.h>
#include <gcal_status.h>

int find_load_file(const char *filename, char **result)
{
        /* find_load_file is a function that will magically load the contents
         * of a XML file into a string. If you want a POSIX only implementation,
	 * have a look in 'utests/utils.c'
         * of a XML file into a string.
         */
	return 1;
}

int main(int argc, char *argv[])
{
        char *super_contact = NULL;
        const char filename[] = "supercontact.xml";
        gcal_t gcal;
        int result;

        gcal = gcal_new(GCONTACT);
        result = gcal_get_authentication(gcal, "username", "password");

         if (find_load_file(filename, &super_contact))
                 exit(-1);

        result = gcal_add_xmlentry(gcal, super_contact, NULL);
        if (result == -1)
                /* Check for errors and print status code */
                printf("Failed adding a new contact! HTTP code: %d"
                "\nmsg: %s\n", gcal_status_httpcode(gcal),
                gcal_status_msg(gcal));

        /* Cleanup */
        free(super_contact);
        gcal_delete(gcal);
        gcal_final_cleanup();

        return 0;
}

