#include "CppUTest/TestHarness.h"

extern "C" {
#include "list.h"
}

TEST_GROUP(SinglyLinkedList) {
	struct list head;
	void setup(void) {
		list_init(&head);
	}
	void teardown() {
	}

	void make_list(struct list *h, int n) {
		for (int i = 0; i < n; i++) {
			struct list *p = (struct list *)malloc(sizeof(*p));
			list_add(p, h);
		}
	}
	void clean_list(struct list *h) {
		struct list *p, *n;
		list_for_each_safe(p, n, h) {
			free(p);
		}
	}
};

TEST(SinglyLinkedList, init_ShouldSetNextItself) {
	struct list tmp;
	head.next = (struct list *)1;
	list_init(&tmp);
	CHECK_EQUAL(&tmp, tmp.next);
}

TEST(SinglyLinkedList, add_ShouldAddNodeNextToHead) {
	struct list node1, node2;
	list_add(&node1, &head); // head->node1->NULL
	list_add(&node2, &head); // head->node2->node1->NULL
	CHECK_EQUAL(&node2, head.next);
	CHECK_EQUAL(&node1, head.next->next);
	CHECK_EQUAL(&head, head.next->next->next);
}

TEST(SinglyLinkedList, add_tail_ShouldAddNodeAtTheEnd) {
	struct list node1, node2;
	list_add_tail(&node1, &head); // head->node1->NULL
	list_add_tail(&node2, &head); // head->node1->node2->NULL
	CHECK_EQUAL(&node1, head.next);
	CHECK_EQUAL(&node2, head.next->next);
	CHECK_EQUAL(&head, head.next->next->next);
}

TEST(SinglyLinkedList, del_ShouldDeleteNodeFromTheList) {
	struct list node1, node2;
	list_add_tail(&node1, &head); // head->node1->NULL
	list_add_tail(&node2, &head); // head->node1->node2->NULL
	CHECK(list_del(&node1, &head) == 0);
	CHECK_EQUAL(&node2, head.next);
	CHECK_EQUAL(&head, head.next->next);
}

TEST(SinglyLinkedList, del_ShouldFailDeleting_WhenThereIsNoMatchingNode) {
	struct list node;
	CHECK(list_del(&node, &head) == -1);
}

TEST(SinglyLinkedList, empty_ShouldReturnTrue_WhenListHasNoNode) {
	CHECK_TRUE(list_empty(&head));
}

TEST(SinglyLinkedList, empty_ShouldReturnFalse_WhenListHasNode) {
	struct list node;
	list_add(&node, &head);
	CHECK_FALSE(list_empty(&head));
}

TEST(SinglyLinkedList, count_ShouldReturnNumberOfNodesInTheList) {
	int n = 13;
	make_list(&head, n);
	CHECK(list_count(&head) == n);
	clean_list(&head);
}

TEST(SinglyLinkedList, count_ShouldReturnZero_WhenNoNodeInTheList) {
	CHECK(list_count(&head) == 0);
}

TEST(SinglyLinkedList, extraTest) {
	int n = 10;
	make_list(&head, n);
	struct list *p = &head;
	for (int i = 0; i < n; i++) {
		CHECK(p->next);
		p = p->next;
	}
	CHECK(p->next == &head);
	clean_list(&head);
}
