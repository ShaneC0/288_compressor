#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Bitmap
{
    int rows;
    int cols;
    short *pixels;
};

struct Node
{
    short intensity;
    struct Node *children[4];
};

struct Tree
{
    struct Node *root;
};

struct Bitmap Bitmap_new(char *fname)
{
    FILE *fp = fopen(fname, "r");
    struct Bitmap bmap;
    char buffer[8];
    int num_lines = 0;
    int i;

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
        num_lines++;

    bmap.rows = (int)sqrt(num_lines);
    bmap.cols = (int)sqrt(num_lines);
    bmap.pixels = (short *)malloc(num_lines * sizeof(short));

    fseek(fp, 0, SEEK_SET);

    for (i = 0; i < num_lines; i++)
        fscanf(fp, "%hd", &bmap.pixels[i]);

    fclose(fp);

    return bmap;
}

void Bitmap_clear(struct Bitmap *b)
{
    if (b->pixels != NULL)
    {
        free(b->pixels);
        b->pixels = NULL;
    }

    b->rows = 0;
    b->cols = 0;
}

void Bitmap_rotate_left(struct Bitmap *b)
{
    int i, j;
    for (i = 0; i < b->rows; i++)
    {
        for(j = i + 1; j < b->cols; j++)
        {
            short temp = b->pixels[i * b->cols + j];
            b->pixels[i * b->cols + j] = b->pixels[j * b->cols + i];
            b->pixels[j * b->cols + i] = temp;
        }
    }

    for(i = 0; i < b->cols; i++)
    {
        for(j = 0; j < b->rows / 2; j++)
        {
            short temp = b->pixels[j * b->cols + i];
            b->pixels[j * b->cols + i] = b->pixels[(b->rows - 1 - j) * b-> cols + i];
            b->pixels[(b->rows - 1 - j) * b->cols + i] = temp;
        }
    }
}

void Bitmap_rotate_right(struct Bitmap *b)
{
    int i, j;
    for (i = 0; i < b->rows; i++)
    {
        for (j = i + 1; j < b->cols; j++)
        {
            short temp = b->pixels[i * b->cols + j];
            b->pixels[i * b->cols + j] = b->pixels[j * b->cols + i];
            b->pixels[j * b->cols + i] = temp;
        }
    }

    for (i = 0; i < b->rows; i++)
    {
        for (j = 0; j < b->cols / 2; j++)
        {
            short temp = b->pixels[i * b->cols + j];
            b->pixels[i * b->cols + j] = b->pixels[i * b->cols + (b->cols - j - 1)];
            b->pixels[i * b->cols + (b->cols - j - 1)] = temp;
        }
    }
}

void Bitmap_flip_vertical(struct Bitmap *b)
{
    int i, j;
    for(i = 0; i < b->cols; i++) {
        for(j = 0; j < b->rows / 2; j++)
        {
            short temp = b->pixels[j * b->rows + i];
            b->pixels[j * b->rows + i] = b->pixels[(b->rows - 1 - j) * b->cols + i];
            b->pixels[(b->rows - 1 - j) * b->cols + i] = temp;
        }
    }
}

void Bitmap_flip_horizontal(struct Bitmap *b)
{
    int i, j;
    for (i = 0; i < b->cols; i++)
    {
        for(j = 0; j < b->rows / 2; j++)
        {
            short temp = b->pixels[i * b->cols + j];
            b->pixels[i * b->cols + j] = b->pixels[i * b->cols + (b->rows - 1 - j)];
            b->pixels[i * b->cols + (b->rows - 1 - j)] = temp;
        }
    }
}

struct Node *Node_new(short intensity)
{
    int i;
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->intensity = intensity;
    for (i = 0; i < 4; i++)
    {
        node->children[i] = NULL;
    }
    return node;
}

struct Node *Tree_from_bitmap(struct Bitmap *b, int row_start, int row_end, int col_start, int col_end)
{
    int i, j, row_mid, col_mid;
    int uniform = 1;
    int test_pixel = b->pixels[row_start * b->cols + col_start];
    struct Node *node;
    for (i = row_start; i <= row_end; i++)
    {
        for (j = col_start; j <= col_end; j++)
        {
            if (b->pixels[i * b->cols + j] != test_pixel)
            {
                uniform = 0;
                break;
            }
        }
        if (!uniform)
        {
            break;
        }
    }

    if (uniform)
    {
        node = Node_new(test_pixel);
    }
    else
    {
        node = Node_new(-1);
        row_mid = (row_start + row_end) / 2;
        col_mid = (col_start + col_end) / 2;
        node->children[0] = Tree_from_bitmap(b, row_start, row_mid, col_start, col_mid);
        node->children[1] = Tree_from_bitmap(b, row_start, row_mid, col_mid + 1, col_end);
        node->children[2] = Tree_from_bitmap(b, row_mid + 1, row_end, col_start, col_mid);
        node->children[3] = Tree_from_bitmap(b, row_mid + 1, row_end, col_mid + 1, col_end);
    }
    return node;
}

struct Tree Tree_new(struct Bitmap *b)
{
    struct Tree tree;
    tree.root = Tree_from_bitmap(b, 0, b->rows - 1, 0, b->cols - 1);
    return tree;
}

void Node_free(struct Node *node)
{
    int i;
    if (node == NULL)
        return;

    for (i = 0; i < 4; i++)
    {
        Node_free(node->children[i]);
    }

    free(node);
}

void Tree_clear(struct Tree *t)
{
    Node_free(t->root);
}

void Node_save(struct Node *node, FILE *fp)
{
    int i;

    if (node == NULL)
        return;

    fprintf(fp, "%d\n", node->intensity);

    for (i = 0; i < 4; i++)
    {
        Node_save(node->children[i], fp);
    }
}

void Tree_save(struct Tree *t, char *fname)
{
    FILE *file = fopen(fname, "w");

    Node_save(t->root, file);

    fclose(file);
}

void Bitmap_print(struct Bitmap *b)
{
    int i, j;
    for (i = 0; i < b->rows; i++)
    {
        for (j = 0; j < b->cols; j++)
        {
            printf("%d ", b->pixels[i * b->cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    struct Bitmap b = Bitmap_new(argv[1]);
    struct Tree t;
    int done = 0;
    char choice;
    if (argc < 2)
        return 1;

    while (!done)
    {
        printf("Options menu:\n");
        printf("1. Rotate Left (counterclockwise)\n");
        printf("2. Rotate Right (clockwise)\n");
        printf("3. Flip Vertical\n");
        printf("4. Flip Horizontal\n");
        printf("5. Save and Quit\n");
        printf("\nYour choice? ");
        scanf("%c", &choice);
        switch (choice)
        {
        case '1':
            Bitmap_rotate_left(&b);
            break;
        case '2':
            Bitmap_rotate_right(&b);
            break;
        case '3':
            Bitmap_flip_vertical(&b);
            break;
        case '4':
            Bitmap_flip_horizontal(&b);
            break;
        case '5':
            t = Tree_new(&b);
            Tree_save(&t, strcat(argv[1], ".compressed"));
            Tree_clear(&t);
            Bitmap_clear(&b);
            done = 1;
            break;
        }
    }
    return 0;
}
