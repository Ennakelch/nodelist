// MIT License

// Copyright(c) 2021 Golden Rockefeller

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :

// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef GOLDENROCKEFELLER_NODE_LIST_HPP
#define GOLDENROCKEFELLER_NODE_LIST_HPP

#include <stdexcept>
#include <iterator>
#include <memory>
#include <utility>
#include <iostream>
#include <sstream>

namespace goldenrockefeller {

template <typename T>
class NodeList {

private:
	class Node {
	public:
		Node* next_node;
		Node* prev_node;

		Node() noexcept : next_node{ nullptr }, prev_node{ nullptr }   {};

		Node(const Node& node) = delete;
		Node(Node&& node) = delete;

		Node& operator=(const Node& node) = delete;
		Node& operator=(const Node&& node) = delete;
	};

	Node before_start_node;
	Node past_end_node;

public:
	using value_type = T;
	using allocator_type = std::allocator<value_type>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	class DataNode : private Node {
	public:
		value_type data;

		DataNode() noexcept : Node() {};

		explicit DataNode(T data) noexcept : Node(), data{ data } {};

		~DataNode() {
			this->detach();
		};

		DataNode(const DataNode& node) = delete;
		DataNode(DataNode&& node) = delete;

		DataNode& operator=(const DataNode& node) = delete;
		DataNode& operator=(DataNode&& node) = delete;

		bool is_attached() const noexcept {
			return bool(this->next_node) && bool(this->prev_node);
		}

		void attach_to(NodeList& list) {
			this->attach_before(&(list.past_end_node));
		};

		void attach_before(Node* node) {
			if (!node) {
				throw std::invalid_argument("The other node must not be null.");
			}
			
			if (!node->prev_node) {
				throw std::invalid_argument("The other node must be attached (previous node is null).");
			}

			this->detach();

			if (node->prev_node) {
				this->next_node = node;
				this->prev_node = node->prev_node;
				node->prev_node->next_node = this;
				node->prev_node = this;
			}
		};

		void attach_before(DataNode* node) {
			this->attach_before(node);
		};

		void attach_after(Node* node) {
			if (!node) {
				throw std::invalid_argument("The other node must not be null.");
			}

			if (!node->next_node) {
				throw std::invalid_argument("The other node must be attached (next node is null).");
			}

			this->detach();

			if (node->next_node) {
				this->next_node = node->next_node;
				this->prev_node = node;
				node->next_node->prev_node = this;
				node->next_node = this;
			}
		};

		void attach_after(DataNode* node) {
			this->attach_after(node);
		};


		void detach() noexcept {
			if (this->prev_node) {
				this->prev_node->next_node = this->next_node;
			}
			if (this->next_node) {
				this->next_node->prev_node = this->prev_node;
			}

			this->next_node = nullptr;
			this->prev_node = nullptr;
		};

		operator T() const noexcept {
			return data;
		}
	};




	template <typename Type>
	class base_iterator
	{

	protected:
		Node* current_node;
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = Type;
		using pointer = Type*;
		using reference = Type&;

		base_iterator() noexcept : current_node{ nullptr } {};
		explicit base_iterator(Node* starting_node) noexcept : current_node(starting_node) {};

		~base_iterator() noexcept {};
		base_iterator(const base_iterator& it) noexcept : current_node{ it.current_node } {};
		base_iterator(base_iterator&& it) noexcept :
			current_node{ it.current_node }
		{
			it.current_node = nullptr;
		};
		base_iterator& operator=(const base_iterator& it) noexcept {
			this->current_node = it.current_node;
			return *this;
		}
		base_iterator& operator=(base_iterator&& it) noexcept {
			this->current_node = it.current_node;
			it.current_node = nullptr;
			return *this;
		}

		bool is_at_nullptr() const noexcept {
			return !this->current_node;
		}

		bool is_at_detached_datanode_no_null_check() const {
			return !bool(this->current_node->next_node) && !bool(this->current_node->prev_node);
		}

		bool is_at_detached_datanode() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			return this->is_at_detached_datanode_no_null_check();
		}

		bool is_at_attached_datanode_no_null_check() const {
			return bool(this->current_node->next_node) && bool(this->current_node->prev_node);
		}

		bool is_at_attached_datanode() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			return this->is_at_attached_datanode_no_null_check();
		}

		bool is_at_datanode_no_null_check() const {
			return (
				this->is_at_attached_datanode_no_null_check()
				|| this->is_at_detached_datanode_no_null_check()
			);
		}

		bool is_at_datanode() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			return this->is_at_datanode_no_null_check();
		}

		bool is_past_the_end_no_null_check() const {
			return !this->current_node->next_node && bool(this->current_node->prev_node);
			
		}

		bool is_past_the_end() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			return this->is_past_the_end_no_null_check();
		}

		bool is_before_the_start_no_null_check() const {
			return !this->current_node->prev_node && bool(this->current_node->next_node);
		}

		bool is_before_the_start() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			return this->is_before_the_start_no_null_check();
		}

		base_iterator& operator++() {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_past_the_end_no_null_check()) {
				throw std::runtime_error("Cannot increment iterator that is past-the-end.");
			}

			this->current_node = this->current_node->next_node;

			return *this;
		}

		base_iterator& operator--() {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_before_the_start_no_null_check()) {
				throw std::runtime_error("Cannot decrement iterator that is before-the-start.");
			}

			this->current_node = this->current_node->prev_node;

			return *this;
		}

		base_iterator operator++(int) {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_past_the_end_no_null_check()) {
				throw std::runtime_error("Cannot increment iterator that is past-the-end.");
			}

			base_iterator it(*this);

			this->current_node = this->current_node->next_node;

			return it;
		}

		base_iterator operator--(int) {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_before_the_start_no_null_check()) {
				throw std::runtime_error("Cannot decrement iterator that is before-the-start.");
			}

			base_iterator it(*this);

			this->current_node = this->current_node->prev_node;

			return it;
		};

		reference operator*() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (!this->is_at_datanode_no_null_check()) {
				throw std::runtime_error("Cannot dereference iterator that is not at data node");
			}

			DataNode* current_data_node = reinterpret_cast<DataNode*>(this->current_node);

			return current_data_node->data;
		};

		pointer operator->() const {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (!this->is_at_datanode_no_null_check()) {
				throw std::runtime_error("Cannot dereference iterator that is not at data node");
			}

			DataNode* current_data_node = reinterpret_cast<DataNode*>(this->current_node);

			return &(current_data_node->data);
		};


		template<typename OtherType>
		bool operator==(const base_iterator<OtherType>& it) const noexcept {
			// Invalid iterators are never equal.
			return this->current_node == it.current_node && !this->is_at_nullptr();
		}

		template<typename OtherType>
		bool operator!=(const base_iterator<OtherType>& it) const noexcept {
			// Invalid iterators are never equal.
			return this->current_node != it.current_node || this->is_at_nullptr();
		}

		operator base_iterator<const value_type>() const noexcept
		{
			return base_iterator<const value_type>(this->current_node);
		}

	};

	template <typename Type>
	struct attachable_iterator : public base_iterator<Type> {

		attachable_iterator() noexcept : base_iterator<Type>() {};
		explicit attachable_iterator(Node* starting_node) noexcept : base_iterator<Type>(starting_node) {};

		void attach_node_before(DataNode& node) {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_before_the_start_no_null_check()) {
				throw std::runtime_error("Cannot attach node before this iterator if this iterator is before-the-start.");
			}

			node.attach_before(this->current_node);
		};

		void attach_node_after(DataNode& node) {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (this->is_past_the_end_no_null_check()) {
				throw std::runtime_error("Cannot attach node after this iterator if this iterator is past-the-end.");
			}

			node.attach_after(this->current_node);
		};

		void detach_current_node_and_increment() {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (!this->is_at_attached_datanode_no_null_check()) {
				throw std::runtime_error("To detach the iterator's current node, the node must be an attached data node.");
			}

			Node* next_node = this->current_node->next_node;
			this->current_node.detach();
			this->current_node = next_node;
		}

		void detach_current_node_and_decrement() {
			if (this->is_at_nullptr()) {
				throw std::runtime_error("The iterator's current node must not be null.");
			}
			if (!this->is_at_attached_datanode_no_null_check()) {
				throw std::runtime_error("To detach the iterator's current node, the node must be an attached data node.");
			}

			Node* prev_node = this->current_node->prev_node;
			this->current_node.detach();
			this->current_node = prev_node;
		}
	};

	using iterator = attachable_iterator<value_type>;
	using const_iterator = base_iterator<const value_type>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;


	NodeList() noexcept {
		this->before_start_node.prev_node = nullptr;
		this->before_start_node.next_node = &(this->past_end_node);
		this->past_end_node.prev_node = &(this->before_start_node);
		this->past_end_node.next_node = nullptr;
	};

	~NodeList() noexcept {
		this->clear();
	};

	NodeList(const NodeList& obj) = delete;
	NodeList(NodeList&& obj) noexcept {
		if (!obj.is_empty()) {
			this->before_start_node.prev_node = obj.before_start_node.prev_node;
			this->before_start_node.next_node = obj.before_start_node.next_node;
			this->past_end_node.prev_node = obj.past_end_node.prev_node;
			this->past_end_node.next_node = obj.past_end_node.next_node;

			obj.before_start_node.prev_node = nullptr;
			obj.before_start_node.next_node = &(obj.past_end_node);
			obj.past_end_node.prev_node = &(obj.before_start_node);
			obj.past_end_node.next_node = nullptr;
		}

		else {
			this->before_start_node.prev_node = nullptr;
			this->before_start_node.next_node = &(this->past_end_node);
			this->past_end_node.prev_node = &(this->before_start_node);
			this->past_end_node.next_node = nullptr;
		}


	};

	NodeList& operator=(NodeList const& obj) = delete;
	NodeList& operator=(NodeList&& obj) noexcept {
		if (!obj.is_empty()) {
			this->before_start_node.prev_node = obj.before_start_node.prev_node;
			this->before_start_node.next_node = obj.before_start_node.next_node;
			this->past_end_node.prev_node = obj.past_end_node.prev_node;
			this->past_end_node.next_node = obj.past_end_node.next_node;

			obj.before_start_node.prev_node = nullptr;
			obj.before_start_node.next_node = &(obj.past_end_node);
			obj.past_end_node.prev_node = &(obj.before_start_node);
			obj.past_end_node.next_node = nullptr;
		}
		else {
			this->before_start_node.prev_node = nullptr;
			this->before_start_node.next_node = &(this->past_end_node);
			this->past_end_node.prev_node = &(this->before_start_node);
			this->past_end_node.next_node = nullptr;
		}

		return *this;
	};

	iterator begin() noexcept {
		return iterator(this->before_start_node.next_node);
	};
	const_iterator begin() const noexcept {
		return const_iterator(this->before_start_node.next_node);
	};
	const_iterator cbegin() const noexcept {
		return this->begin();
	};
	iterator end() noexcept {
		return iterator(&(this->past_end_node));
	};
	const_iterator end() const noexcept {
		return const_iterator(&(this->past_end_node));
	};
	const_iterator cend() const noexcept {
		return this->end();
	};

	reverse_iterator rbegin() noexcept {
		return reverse_iterator(iterator(this->past_end_node.prev_node));
	};
	const_reverse_iterator rbegin() const noexcept {
		return reverse_iterator(const_iterator(this->past_end_node.prev_node));
	};
	const_reverse_iterator crbegin() const noexcept {
		return this->rbegin();
	};
	reverse_iterator rend() noexcept {
		return reverse_iterator(iterator(&(this->before_start_node)));
	};
	const_reverse_iterator rend() const noexcept {
		return reverse_iterator(const_iterator(&(this->before_start_node)));
	};
	const_reverse_iterator crend() const noexcept {
		return this->rend();
	};


	bool is_empty() const noexcept {
		return (
			this->before_start_node.next_node == &(this->past_end_node)
			&& this->past_end_node.prev_node == &(this->before_start_node)
		);
	};

	size_type size() const noexcept {

		size_type size{ 0 };
		Node* node{ this->before_start_node.next_node };

		while (node != &(this->past_end_node)) {
			node = node->next_node;
			size++;
		}

		return size;
	};


	void clear() noexcept {
		Node* node{ &(this->before_start_node) };

		// Manually detach all nodes in the list.
		while (node) {
			Node* next_node = node->next_node;
			node->next_node = nullptr;
			node->prev_node = nullptr;
			node = next_node;
		}

		// Reattach this list's past-the-end and before-the-start nodes.
		this->before_start_node.prev_node = nullptr;
		this->before_start_node.next_node = &(this->past_end_node);
		this->past_end_node.prev_node = &(this->before_start_node);
		this->past_end_node.next_node = nullptr;
	};
};

} // namespace goldenrockefeller

#endif