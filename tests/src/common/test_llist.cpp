/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"

#include "libmcu/llist.h"

TEST_GROUP(DoublyLinkedList) {
	struct llist llist;

	void setup(void) {
		llist_init(&llist);
	}
	void teardown() {
	}
};

TEST(DoublyLinkedList, init_ShouldMakeCircularList) {
	POINTERS_EQUAL(&llist, llist.next);
	POINTERS_EQUAL(&llist, llist.prev);

	DEFINE_LLIST_HEAD(statically_defined_list);
	POINTERS_EQUAL(&statically_defined_list, statically_defined_list.next);
	POINTERS_EQUAL(&statically_defined_list, statically_defined_list.prev);
}

TEST(DoublyLinkedList, add_ShouldDoNothing_WhenNullPointersGiven) {
	llist_add(&llist, NULL);
	CHECK(llist_empty(&llist) == true);
}

TEST(DoublyLinkedList, add_ShouldAdd_WhenSingleNodeGiven) {
	struct llist node1;
	llist_add(&node1, &llist);
	CHECK(llist_empty(&llist) == false);
	POINTERS_EQUAL(&llist, node1.next);
	POINTERS_EQUAL(&llist, node1.prev);
	POINTERS_EQUAL(&node1, llist.next);
	POINTERS_EQUAL(&node1, llist.prev);
}

TEST(DoublyLinkedList, add_ShouldAdd_WhenMultipleNodesGiven) {
	struct llist node1, node2, node3;
	llist_add(&node1, &llist);
	llist_add(&node2, &llist);
	llist_add(&node3, &llist);
	POINTERS_EQUAL(&llist, node1.next);
	POINTERS_EQUAL(&node2, node1.prev);
	POINTERS_EQUAL(&node1, node2.next);
	POINTERS_EQUAL(&node3, node2.prev);
	POINTERS_EQUAL(&node2, node3.next);
	POINTERS_EQUAL(&llist, node3.prev);
	POINTERS_EQUAL(&node3, llist.next);
	POINTERS_EQUAL(&node1, llist.prev);
}

TEST(DoublyLinkedList, add_tail_ShouldAddReverse) {
	struct llist node1, node2, node3;
	llist_add_tail(&node1, &llist);
	llist_add_tail(&node2, &llist);
	llist_add_tail(&node3, &llist);
	POINTERS_EQUAL(&node2, node1.next);
	POINTERS_EQUAL(&llist, node1.prev);
	POINTERS_EQUAL(&node3, node2.next);
	POINTERS_EQUAL(&node1, node2.prev);
	POINTERS_EQUAL(&llist, node3.next);
	POINTERS_EQUAL(&node2, node3.prev);
	POINTERS_EQUAL(&node1, llist.next);
	POINTERS_EQUAL(&node3, llist.prev);
}

TEST(DoublyLinkedList, add_tail_ShouldDoNothing_WhenNullPointersGiven) {
	llist_add_tail(&llist, NULL);
	CHECK(llist_empty(&llist) == true);
}

TEST(DoublyLinkedList, del_ShouldDoNothing_WhenNullPointerGiven) {
	struct llist node1;
	llist_add(&node1, &llist);
	llist_del(NULL);
	CHECK(llist_empty(&llist) == false);
}

TEST(DoublyLinkedList, del_ShouldDeleteGivenNode_WhenSingleNodeExists) {
	struct llist node1;
	llist_add(&node1, &llist);
	llist_del(&node1);
	CHECK(llist_empty(&llist) == true);
	POINTERS_EQUAL(LLIST_POISON_NEXT, node1.next);
	POINTERS_EQUAL(LLIST_POISON_PREV, node1.prev);
}

TEST(DoublyLinkedList, del_ShouldDeleteGivenNode_WhenMultipleNodesExist) {
	struct llist node1, node2, node3;
	llist_add_tail(&node1, &llist);
	llist_add_tail(&node2, &llist);
	llist_add_tail(&node3, &llist);
	llist_del(&node2);
	POINTERS_EQUAL(&node3, node1.next);
	POINTERS_EQUAL(&llist, node1.prev);
	POINTERS_EQUAL(LLIST_POISON_NEXT, node2.next);
	POINTERS_EQUAL(LLIST_POISON_PREV, node2.prev);
	POINTERS_EQUAL(&llist, node3.next);
	POINTERS_EQUAL(&node1, node3.prev);
	POINTERS_EQUAL(&node1, llist.next);
	POINTERS_EQUAL(&node3, llist.prev);
}

TEST(DoublyLinkedList, empty_ShouldReturnFalse_WhenNullPointerGiven) {
	CHECK(llist_empty(NULL) == false);
}

TEST(DoublyLinkedList, empty_ShouldReturnTrue_WhenEmpty) {
	CHECK(llist_empty(&llist) == true);
}

TEST(DoublyLinkedList, count_ShouldReturnZero_WhenNoNodesInList) {
	LONGS_EQUAL(0, llist_count(&llist));
}

TEST(DoublyLinkedList, count_ShouldReturnNumberOfNodesInTheList) {
	struct llist node1, node2, node3;
	LONGS_EQUAL(0, llist_count(&llist));
	llist_add(&node1, &llist);
	LONGS_EQUAL(1, llist_count(&llist));
	llist_add(&node2, &llist);
	LONGS_EQUAL(2, llist_count(&llist));
	llist_add(&node3, &llist);
	LONGS_EQUAL(3, llist_count(&llist));
	llist_del(&node1);
	LONGS_EQUAL(2, llist_count(&llist));
}
