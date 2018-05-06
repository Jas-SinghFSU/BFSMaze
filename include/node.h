#ifndef NODE_H
#define NODE_H


class node
{
    public:

        node* next;
        int i;
        int j;

        node();
        node(int, int);
        virtual ~node();

    protected:

    private:
};

#endif // NODE_H
