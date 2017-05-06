#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct struct_linked_list_t
{
    char *s_objName;
    double d_value;
    struct struct_linked_list_t *p_next;
} linked_list_t;

static void freeList(linked_list_t *p_head)
{
    linked_list_t *p_cur, *p_next;
    p_cur = p_head;
    while (p_cur != NULL)
    {
        p_next = p_cur->p_next;
        free(p_cur->s_objName);
        free(p_cur);
        p_cur = p_next;
    }
}

static int loadFile(const char *s_filename, linked_list_t **pp_head, unsigned int *pu_count)
{
    char ca_buf[256];
    char *s_objName, *p_chrPos;
    double d_val;
    FILE *pf_data;
    linked_list_t *p_head = NULL, *p_cur, **pp_next;
    unsigned int u_lineCount = 0, u_elementCount = 0;

    pf_data = fopen(s_filename, "r");

    if (!pf_data)
    {
        fprintf(stderr, "[ERROR] Cannot open %s\n", s_filename);
        return 1;
    }

    while (fgets(ca_buf, sizeof(ca_buf), pf_data))
    {
        u_lineCount += 1;
        p_chrPos = strchr(ca_buf, '\r');
        if (p_chrPos != NULL)
            *p_chrPos = '\0';
        p_chrPos = strchr(ca_buf, '\n');
        if (p_chrPos != NULL)
            *p_chrPos = '\0';
        if (strlen(ca_buf) == 0)
            continue;
        p_chrPos = strchr(ca_buf, ',');
        if (p_chrPos == NULL)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, no seperator.\n", s_filename, u_lineCount);
            fclose(pf_data);
            freeList(p_head);
            return 3;
        }
        *p_chrPos = '\0';
        p_chrPos += 1;
        if ((s_objName = strdup(ca_buf)) == NULL)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, s_objName NULL (requesting size %u).\n", s_filename, u_lineCount, (unsigned int)(strlen(ca_buf) + 1));
            fclose(pf_data);
            freeList(p_head);
            return 4;
        }
        if (sscanf(p_chrPos, "%lf", &d_val) != 1)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, cannot parse value.\n", s_filename, u_lineCount);
            fclose(pf_data);
            freeList(p_head);
            free(s_objName);
            return 5;
        }
        pp_next = &p_head;
        while ((*pp_next) != NULL)
        {
            if (d_val >= (*pp_next)->d_value)
                break;
            pp_next = &((*pp_next)->p_next);
        }
        p_cur = (linked_list_t *)malloc(sizeof(*p_cur));
        if (!p_cur)
        {
            fprintf(stderr, "[ERROR] Error in %s, line %u, malloc failed (requesting size %u).\n", s_filename, u_lineCount, (unsigned int)sizeof(*p_cur));
            fclose(pf_data);
            freeList(p_head);
            free(s_objName);
            return 6;
        }
        p_cur->s_objName = s_objName;
        p_cur->d_value = d_val;
        p_cur->p_next = *pp_next;
        *pp_next = p_cur;
        u_elementCount += 1;
    }

    if (!feof(pf_data))
    {
        fprintf(stderr, "[ERROR] Cannot read %s\n", s_filename);
        fclose(pf_data);
        return 2;
    }

    fclose(pf_data);
    if (pp_head != NULL)
        *pp_head = p_head;
    else
        freeList(p_head);
    if (pu_count != NULL)
        *pu_count = u_elementCount;
    return 0;
}

typedef struct
{
    char *s_objName;
    double d_value;
} object_t;

static object_t *list2array(linked_list_t *p_head, unsigned int u_length)
{
    linked_list_t *p_cur, *p_next;
    object_t *pb_object = NULL;
    unsigned int i = 0;

    pb_object = (object_t *)malloc(sizeof(*pb_object) * u_length);
    if (pb_object == NULL)
        return 0;

    p_cur = p_head;
    while (p_cur != NULL)
    {
        p_next = p_cur->p_next;
        pb_object[i].s_objName = p_cur->s_objName;
        pb_object[i].d_value = p_cur->d_value;
        free(p_cur);
        p_cur = p_next;
        i += 1;
    }

    return pb_object;
}

static int cmpObj(const void *a, const void *b)
{
    object_t *c, *d;

    c = (object_t *)a;
    d = (object_t *)b;

    if (c->d_value > d->d_value)
        return 1;
    else if (c->d_value < d->d_value)
        return -1;
    else
        return 0;
}

static void usage(const char *s_exeName)
{
    fprintf(stderr, "[USAGE] %s <filename> [<num of clusters> [<max iteration>]]\n", s_exeName);
    fprintf(stderr, "[USAGE] File format: OBJ_NAME,VALUE\n");
    fprintf(stderr, "[USAGE] Note num of clusters must > 0\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char ca_bBuf[32];
    double d_mindis, d_minVal, d_maxVal, d_dis, d_valsum, d_newCenter;
    linked_list_t *p_head;
    object_t *pb_object;
    double *pb_kpoints;
    unsigned int *pb_myClusters;
    unsigned int u_length, u_nclusters, i, j, ui_closestCluster = 0, u_nclusterMember, u_kpointMoves, u_maxIteration, u_iteration;
    int i_retCode, i_printf = 0;
    ;

    if (argc < 2)
        usage(argv[0]);
    else if (argc == 2)
    {
        u_nclusters = 0;
        u_maxIteration = 0;
    }
    else if (argc == 3)
    {
        if (sscanf(argv[2], "%u", &u_nclusters) != 1)
            usage(argv[0]);
        u_maxIteration = 0;
    }
    else if (argc == 4)
    {
        if (sscanf(argv[2], "%u", &u_nclusters) != 1)
            usage(argv[0]);
        else if (sscanf(argv[3], "%u", &u_maxIteration) != 1)
            usage(argv[0]);
    }
    else
        usage(argv[0]);

    if ((i_retCode = loadFile(argv[1], &p_head, &u_length) != 0))
    {
        fprintf(stderr, "[ERROR] loadFile(%s, %p, %p) got %d\n", argv[1], &p_head, &u_length, i_retCode);
        return 1;
    }

    if (u_length == 0)
    {
        fprintf(stderr, "[INFO] 0 values, exiting\n");
        freeList(p_head);
        return 1;
    }

    if (u_nclusters == 0)
    {
        u_nclusters = (unsigned int)ceil(sqrt(u_length));
    }
    if (u_maxIteration == 0)
    {
        u_maxIteration = u_nclusters * u_nclusters * u_nclusters;
    }

    if (u_nclusters >= u_length)
    {
        fprintf(stderr, "[INFO] Number of clusters is greater than number objects, exiting\n");
        freeList(p_head);
        return 1;
    }

    pb_kpoints = (double *)malloc(sizeof(*pb_kpoints) * u_nclusters);
    if (pb_kpoints == NULL)
    {
        fprintf(stderr, "[ERROR] pb_kpoints, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*pb_kpoints) * u_nclusters));
        freeList(p_head);
        return 1;
    }

    pb_myClusters = (unsigned int *)malloc(sizeof(*pb_myClusters) * u_length);
    if (pb_kpoints == NULL)
    {
        fprintf(stderr, "[ERROR] pb_myClusters, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*pb_myClusters) * u_length));
        freeList(p_head);
        free(pb_kpoints);
        return 1;
    }

    if ((pb_object = list2array(p_head, u_length)) == NULL)
    {
        fprintf(stderr, "[ERROR] list2array(%p, %u) got NULL\n", p_head, u_length);
        freeList(p_head);
        free(pb_kpoints);
        return 1;
    }

    qsort(pb_object, u_length, sizeof(*pb_object), cmpObj);

    //for (i = 0; i < u_length; i += 1)
    //    fprintf(stderr, "[DEBUG] [%u]\t%s,%lf\n", i, pb_object[i].s_objName, pb_object[i].d_value);
    d_maxVal = pb_object[u_length - 1].d_value;
    d_minVal = pb_object[0].d_value;
    fprintf(stderr, "[DEBUG] n = %u, min = %lf, max = %lf\n", u_length, d_minVal, d_maxVal);

    for (i = 0; i < u_nclusters; i += 1)
        pb_kpoints[i] = pb_object[(u_length * (i + 1)) / u_nclusters].d_value;

    u_iteration = 0;
    do
    {
        for (i = 0; i < u_length; i += 1)
        {
            d_mindis = d_maxVal - d_minVal;
            for (j = 0; j < u_nclusters; j += 1)
            {
                d_dis = pb_object[i].d_value - pb_kpoints[j];
                if (d_dis < 0)
                    d_dis *= -1;
                if (d_dis < d_mindis)
                {
                    d_mindis = d_dis;
                    ui_closestCluster = j;
                }
            }
            pb_myClusters[i] = ui_closestCluster;
        }

        u_kpointMoves = 0;
        for (i = 0; i < u_nclusters; i += 1)
        {
            u_nclusterMember = 0;
            d_valsum = 0.0;
            for (j = 0; j < u_length; j += 1)
            {
                if (pb_myClusters[j] == i)
                {
                    d_valsum += pb_object[j].d_value;
                    u_nclusterMember += 1;
                }
            }
            if (u_nclusterMember > 0)
            {
                d_newCenter = d_valsum / u_nclusterMember;
                if (d_newCenter != pb_kpoints[i])
                {
                    pb_kpoints[i] = d_newCenter;
                    u_kpointMoves += 1;
                }
            }
            else
                pb_kpoints[i] = pb_object[(u_length * (i + 1)) / u_nclusters].d_value;
        }
        u_iteration += 1;
        if (i_printf >= 0)
        {
            memset(ca_bBuf, '\b', i_printf);
            ca_bBuf[i_printf] = '\0';
            printf("%s", ca_bBuf);
        }
        i_printf = printf("%u/%u", u_iteration, u_maxIteration);
    } while (u_kpointMoves != 0 && u_iteration < u_maxIteration);
    printf("\n");

    for (i = 0; i < u_nclusters; i += 1)
    {
        printf("=== Cluster %u ===\n", i + 1);
        for (j = 0; j < u_length; j += 1)
            if (pb_myClusters[j] == i)
                printf("%s,%.15lf,%u\n", pb_object[j].s_objName, pb_object[j].d_value, i);

        printf("=== Cluster %u ===\n\n", i + 1);
    }

    for (i = 0; i < u_length; i += 1)
        free(pb_object[i].s_objName);
    free(pb_object);
    free(pb_kpoints);
    free(pb_myClusters);

    return 0;
}
