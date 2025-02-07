import java.util.*;

class Node {
    boolean isLeaf;
    List<Float> keys;
    List<Node> children;
    List<Record> records;
    Node next; // For linked leaf nodes (range search optimization)

    Node(boolean isLeaf) {
        this.isLeaf = isLeaf;
        this.keys = new ArrayList<>();
        this.next = null; // Initialize as null
        if (isLeaf) {
            this.records = new ArrayList<>();
        } else {
            this.children = new ArrayList<>();
        }
    }
}

public class BPlusTree {
    private static final int ORDER = 4; // B+ Tree Order
    private Node root;

    public BPlusTree() {
        root = new Node(true); // Start with an empty leaf node
    }

    public void insert(float key, Record record) {
        Node leaf = findLeafNode(key);
        leaf.keys.add(key);
        leaf.records.add(record);
        sortLeaf(leaf);

        if (leaf.keys.size() > ORDER - 1) {
            splitLeafNode(leaf);
        }
    }

    private Node findLeafNode(float key) {
        Node current = root;
        while (!current.isLeaf) {
            int i = 0;
            while (i < current.keys.size() && key > current.keys.get(i)) i++;
            current = current.children.get(i);
        }
        return current;
    }

    private void sortLeaf(Node leaf) {
        List<Map.Entry<Float, Record>> sorted = new ArrayList<>();
        for (int i = 0; i < leaf.keys.size(); i++) {
            sorted.add(Map.entry(leaf.keys.get(i), leaf.records.get(i)));
        }
        sorted.sort(Map.Entry.comparingByKey());

        leaf.keys.clear();
        leaf.records.clear();
        for (Map.Entry<Float, Record> entry : sorted) {
            leaf.keys.add(entry.getKey());
            leaf.records.add(entry.getValue());
        }
    }

    private void splitLeafNode(Node leaf) {
        Node newLeaf = new Node(true);
        int mid = leaf.keys.size() / 2;

        newLeaf.keys.addAll(leaf.keys.subList(mid, leaf.keys.size()));
        newLeaf.records.addAll(leaf.records.subList(mid, leaf.records.size()));

        leaf.keys = new ArrayList<>(leaf.keys.subList(0, mid));
        leaf.records = new ArrayList<>(leaf.records.subList(0, mid));

        newLeaf.next = leaf.next;
        leaf.next = newLeaf; // Maintain linked leaf nodes

        if (leaf == root) {
            Node newRoot = new Node(false);
            newRoot.keys.add(newLeaf.keys.get(0));
            newRoot.children.add(leaf);
            newRoot.children.add(newLeaf);
            root = newRoot;
        }
    }

    // ✅ Implementing Range Search
    public List<Record> rangeSearch(float min, float max) {
        Node leaf = findLeafNode(min);
        List<Record> results = new ArrayList<>();

        while (leaf != null) {
            for (int i = 0; i < leaf.keys.size(); i++) {
                if (leaf.keys.get(i) >= min && leaf.keys.get(i) <= max) {
                    results.add(leaf.records.get(i));
                }
            }
            leaf = leaf.next; // Move to next linked leaf node
        }
        return results;
    }
}
