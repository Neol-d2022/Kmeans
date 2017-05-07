#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct struct_linked_list_t
{
    char *s_objName;
    double *pb_d_value;
    struct struct_linked_list_t *p_next;
    unsigned int u_valLength;
    int i_randVal;
} linked_list_t;

static void freeList(linked_list_t *p_head)
{
    linked_list_t *p_cur, *p_next;
    p_cur = p_head;
    while (p_cur != NULL)
    {
        p_next = p_cur->p_next;
        free(p_cur->s_objName);
        free(p_cur->pb_d_value);
        free(p_cur);
        p_cur = p_next;
    }
}

static int strSplit(const char *s_str, char ***ppb_s_str, unsigned int *pu_vlength)
{
    const char *p_chrPos, *p_prevPos;
    char **pb_s_str;
    size_t z_strLen;
    unsigned int u_count, i, j;

    u_count = 0;
    p_chrPos = s_str;
    while ((p_chrPos = strchr(p_chrPos, ',')) != NULL)
    {
        u_count += 1;
        p_chrPos += 1;
    }
    u_count += 1;
    if (pu_vlength)
        *pu_vlength = u_count;

    pb_s_str = (char **)malloc(sizeof(*pb_s_str) * u_count);
    if (pb_s_str == NULL)
    {
        fprintf(stderr, "[ERROR] Error pb_s_str(count %u) malloc failed (requesting size %u).\n", u_count, (unsigned int)(sizeof(*pb_s_str) * u_count));
        return 1;
    }

    i = 0;
    p_chrPos = s_str;
    p_prevPos = s_str;
    while ((p_chrPos = strchr(p_chrPos, ',')) != NULL)
    {
        z_strLen = (size_t)p_chrPos - (size_t)p_prevPos + 1;
        pb_s_str[i] = (char *)malloc(z_strLen);
        if (pb_s_str[i] == NULL)
        {
            fprintf(stderr, "[ERROR] Error pb_s_str[%u] malloc failed (requesting size %u).\n", i, (unsigned int)z_strLen);
            for (j = 0; j < i; j += 1)
                free(pb_s_str[j]);
            free(pb_s_str);
            return 2;
        }
        memcpy(pb_s_str[i], p_prevPos, z_strLen - 1);
        pb_s_str[i][z_strLen - 1] = '\0';
        p_prevPos = (p_chrPos += 1);
        i += 1;
    }
    z_strLen = strlen(p_prevPos) + 1;
    pb_s_str[i] = (char *)malloc(z_strLen);
    if (pb_s_str[i] == NULL)
    {
        fprintf(stderr, "[ERROR] Error pb_s_str[%u] malloc failed (requesting size %u).\n", i, (unsigned int)z_strLen);
        for (j = 0; j < i; j += 1)
            free(pb_s_str[j]);
        free(pb_s_str);
        return 3;
    }
    memcpy(pb_s_str[i], p_prevPos, z_strLen - 1);
    pb_s_str[i][z_strLen - 1] = '\0';

    if (ppb_s_str)
        *ppb_s_str = pb_s_str;
    else
    {
        for (j = 0; j < u_count; j += 1)
            free(pb_s_str[j]);
        free(pb_s_str);
    }

    return 0;
}

static int loadFile(const char *s_filename, linked_list_t **pp_head, unsigned int *pu_count, unsigned int u_valLength)
{
    char ca_buf[1024];
    char *s_objName, *p_chrPos, **pb_s_values;
    double *pb_d_val;
    FILE *pf_data;
    linked_list_t *p_head = NULL, *p_cur, **pp_next;
    unsigned int u_lineCount = 0, u_elementCount = 0, u_vlength, i;
    int i_retCode, i_randVal;

    srand(time(0));
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
        if ((i_retCode = strSplit(ca_buf, &pb_s_values, &u_vlength)) != 0)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, strSplit got %i.\n", s_filename, u_lineCount, i_retCode);
            fclose(pf_data);
            freeList(p_head);
            return 7;
        }
        if (u_vlength != u_valLength + 1)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, CSV value length mismatch, expecting %u, got %u.\n", s_filename, u_lineCount, u_valLength + 1, u_vlength);
            fclose(pf_data);
            freeList(p_head);
            for (i = 0; i < u_vlength; i += 1)
                free(pb_s_values[i]);
            free(pb_s_values);
            return 8;
        }
        pb_d_val = (double *)malloc(sizeof(*pb_d_val) * u_valLength);
        if (pb_d_val == NULL)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, malloc failed (requesting size %u).\n", s_filename, u_lineCount, (unsigned int)(sizeof(*pb_d_val) * u_valLength));
            fclose(pf_data);
            freeList(p_head);
            for (i = 0; i < u_vlength; i += 1)
                free(pb_s_values[i]);
            free(pb_s_values);
            return 9;
        }
        s_objName = pb_s_values[0];
        /*if ((s_objName = strdup(ca_buf)) == NULL)
        {
            fprintf(stderr, "[ERROR] File format error in %s, line %u, s_objName NULL (requesting size %u).\n", s_filename, u_lineCount, (unsigned int)(strlen(ca_buf) + 1));
            fclose(pf_data);
            freeList(p_head);
            for (i = 0; i < u_vlength; i += 1)
                free(pb_s_values[i]);
            free(pb_s_values);
            return 4;
        }*/
        for (i = 0; i < u_valLength; i += 1)
        {
            if (sscanf(pb_s_values[i + 1], "%lf", pb_d_val + i) != 1)
            {
                fprintf(stderr, "[ERROR] File format error in %s, line %u, cannot parse value %s.\n", s_filename, u_lineCount, pb_s_values[i + 1]);
                fclose(pf_data);
                freeList(p_head);
                free(s_objName);
                for (i = 0; i < u_vlength; i += 1)
                    free(pb_s_values[i]);
                free(pb_s_values);
                free(pb_d_val);
                return 5;
            }
        }
        i_randVal = rand();
        pp_next = &p_head;
        while ((*pp_next) != NULL)
        {
            if (i_randVal >= (*pp_next)->i_randVal)
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
            for (i = 0; i < u_vlength; i += 1)
                free(pb_s_values[i]);
            free(pb_s_values);
            free(pb_d_val);
            return 6;
        }
        p_cur->s_objName = s_objName;
        p_cur->pb_d_value = pb_d_val;
        p_cur->p_next = *pp_next;
        p_cur->u_valLength = u_valLength;
        p_cur->i_randVal = i_randVal;
        *pp_next = p_cur;
        u_elementCount += 1;
        for (i = 1; i < u_vlength; i += 1)
            free(pb_s_values[i]);
        free(pb_s_values);
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
    double *pb_d_value;
    unsigned int u_valLength;
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
        pb_object[i].pb_d_value = p_cur->pb_d_value;
        pb_object[i].u_valLength = p_cur->u_valLength;
        free(p_cur);
        p_cur = p_next;
        i += 1;
    }

    return pb_object;
}

static void usage(const char *s_exeName)
{
    fprintf(stderr, "[USAGE] %s <filename> <values length> [<num of clusters> [<max iteration>]]\n", s_exeName);
    fprintf(stderr, "[USAGE] File format: OBJ_NAME,VALUE\n");
    fprintf(stderr, "[USAGE] Note num of clusters must > 0\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char ca_bBuf[32];
    double d_mindis /*, d_minVal, d_maxVal*/, d_dis, d_diff, *p_d_valsum, *p_d_newCenter;
    linked_list_t *p_head;
    object_t *pb_object;
    double **ppb_d_kpoints;
    unsigned int *pb_myClusters;
    unsigned int u_objLength, u_nclusters, i, j, k, ui_closestCluster = 0, u_nclusterMember, u_kpointMoves, u_maxIteration, u_iteration, u_valLength;
    int i_retCode, i_printf = 0;

    if (argc < 3)
        usage(argv[0]);
    else if (argc == 3)
    {
        if (sscanf(argv[2], "%u", &u_valLength) != 1)
            usage(argv[0]);
        u_nclusters = 0;
        u_maxIteration = 0;
    }
    else if (argc == 4)
    {
        if (sscanf(argv[2], "%u", &u_valLength) != 1)
            usage(argv[0]);
        else if (sscanf(argv[3], "%u", &u_nclusters) != 1)
            usage(argv[0]);
        u_maxIteration = 0;
    }
    else if (argc == 5)
    {
        if (sscanf(argv[2], "%u", &u_valLength) != 1)
            usage(argv[0]);
        else if (sscanf(argv[3], "%u", &u_nclusters) != 1)
            usage(argv[0]);
        else if (sscanf(argv[4], "%u", &u_maxIteration) != 1)
            usage(argv[0]);
    }
    else
        usage(argv[0]);

    if (u_valLength == 0)
        usage(argv[0]);

    if ((p_d_valsum = (double *)malloc(sizeof(*p_d_valsum) * u_valLength)) == NULL)
    {
        fprintf(stderr, "[ERROR] p_d_valsum, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*p_d_valsum) * u_valLength));
        return 1;
    }
    if ((p_d_newCenter = (double *)malloc(sizeof(*p_d_newCenter) * u_valLength)) == NULL)
    {
        fprintf(stderr, "[ERROR] p_d_newCenter, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*p_d_newCenter) * u_valLength));
        free(p_d_valsum);
        return 1;
    }

    if ((i_retCode = loadFile(argv[1], &p_head, &u_objLength, u_valLength) != 0))
    {
        fprintf(stderr, "[ERROR] loadFile(%s, %p, %p) got %d\n", argv[1], &p_head, &u_objLength, i_retCode);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    if (u_objLength == 0)
    {
        fprintf(stderr, "[INFO] 0 values, exiting\n");
        freeList(p_head);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    if (u_nclusters == 0)
    {
        u_nclusters = (unsigned int)ceil(sqrt(u_objLength));
    }
    if (u_maxIteration == 0)
    {
        u_maxIteration = u_nclusters * u_nclusters * u_nclusters;
    }

    if (u_nclusters > u_objLength)
    {
        fprintf(stderr, "[INFO] Number of clusters is greater than number objects, exiting\n");
        freeList(p_head);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    ppb_d_kpoints = (double **)malloc(sizeof(*ppb_d_kpoints) * u_nclusters);
    if (ppb_d_kpoints == NULL)
    {
        fprintf(stderr, "[ERROR] ppb_d_kpoints, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*ppb_d_kpoints) * u_nclusters));
        freeList(p_head);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    for (i = 0; i < u_nclusters; i += 1)
    {
        if ((ppb_d_kpoints[i] = (double *)malloc(sizeof(**ppb_d_kpoints) * u_valLength)) == NULL)
        {
            fprintf(stderr, "[ERROR] ppb_d_kpoints[%u], malloc failed (requesting size %u)\n", i, (unsigned int)(sizeof(**ppb_d_kpoints) * u_valLength));
            for (j = 0; j < i; j += 1)
                free(ppb_d_kpoints[i]);
            free(ppb_d_kpoints);
            freeList(p_head);
            free(p_d_valsum);
            free(p_d_newCenter);
            return 1;
        }
    }

    pb_myClusters = (unsigned int *)malloc(sizeof(*pb_myClusters) * u_objLength);
    if (ppb_d_kpoints == NULL)
    {
        fprintf(stderr, "[ERROR] pb_myClusters, malloc failed (requesting size %u)\n", (unsigned int)(sizeof(*pb_myClusters) * u_objLength));
        freeList(p_head);
        for (j = 0; j < u_nclusters; j += 1)
            free(ppb_d_kpoints[i]);
        free(ppb_d_kpoints);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    if ((pb_object = list2array(p_head, u_objLength)) == NULL)
    {
        fprintf(stderr, "[ERROR] list2array(%p, %u) got NULL\n", p_head, u_objLength);
        freeList(p_head);
        for (j = 0; j < u_nclusters; j += 1)
            free(ppb_d_kpoints[i]);
        free(ppb_d_kpoints);
        free(p_d_valsum);
        free(p_d_newCenter);
        return 1;
    }

    /*for (i = 0; i < u_objLength; i += 1)
        fprintf(stderr, "[DEBUG] [%u]\t%s,%lf\n", i, pb_object[i].s_objName, pb_object[i].d_value);
    d_maxVal = pb_object[u_objLength - 1].d_value;
    d_minVal = pb_object[0].d_value;
    fprintf(stderr, "[DEBUG] n = %u, min = %lf, max = %lf\n", u_objLength, d_minVal, d_maxVal);*/

    for (i = 0; i < u_nclusters; i += 1)
        for (k = 0; k < u_valLength; k += 1)
            ppb_d_kpoints[i][k] = pb_object[(j = ((u_objLength * (i + 1)) / u_nclusters)) < u_valLength ? j : (u_valLength - 1)].pb_d_value[k];

    u_iteration = 0;
    do
    {
        for (i = 0; i < u_objLength; i += 1)
        {
            d_mindis = DBL_MAX;
            for (j = 0; j < u_nclusters; j += 1)
            {
                d_dis = 0.0;
                for (k = 0; k < u_valLength; k += 1)
                {
                    d_diff = pb_object[i].pb_d_value[k] - ppb_d_kpoints[j][k];
                    d_dis = d_diff * d_diff;
                }
                d_dis = sqrt(d_dis);

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
            for (k = 0; k < u_valLength; k += 1)
                p_d_valsum[k] = 0.0;
            for (j = 0; j < u_objLength; j += 1)
            {
                if (pb_myClusters[j] == i)
                {
                    for (k = 0; k < u_valLength; k += 1)
                        p_d_valsum[k] += pb_object[j].pb_d_value[k];
                    u_nclusterMember += 1;
                }
            }
            if (u_nclusterMember > 0)
            {
                for (k = 0; k < u_valLength; k += 1)
                    p_d_newCenter[k] = p_d_valsum[k] / u_nclusterMember;
                for (k = 0; k < u_valLength; k += 1)
                    if (p_d_newCenter[k] != ppb_d_kpoints[i][k])
                        break;
                if (k != u_valLength)
                {
                    for (k = 0; k < u_valLength; k += 1)
                        ppb_d_kpoints[i][k] = p_d_newCenter[k];
                    u_kpointMoves += 1;
                }
            }
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
        for (j = 0; j < u_objLength; j += 1)
            if (pb_myClusters[j] == i)
            {
                printf("%s\n", pb_object[j].s_objName);
                /*for (k = 0; k < u_valLength; k += 1)
                    printf(",%.15lf", pb_object[j].pb_d_value[k]);
                printf(",%u\n", i);*/
            }

        printf("=== Cluster %u ===\n\n", i + 1);
    }

    for (i = 0; i < u_objLength; i += 1)
        free(pb_object[i].s_objName);
    free(pb_object);
    for (j = 0; j < u_nclusters; j += 1)
        free(ppb_d_kpoints[j]);
    free(ppb_d_kpoints);
    free(pb_myClusters);
    free(p_d_valsum);
    free(p_d_newCenter);

    return 0;
}
