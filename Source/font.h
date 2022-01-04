#pragma once

#ifdef FONT_STATIC
#define FONT_PORT
#else
#ifdef FONT_DLL
#define FONT_PORT __declspec(dllexport)
#else
#define FONT_PORT __declspec(dllimport)
#endif
#endif

#include <cstdint>
#include <stdint.h>
#include <cstddef>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

namespace Font {
template <typename T>
class FONT_PORT LinkedList {
private:
    class ListNode {
    private:
        T _value;
        ListNode *_next, *_prev;
        friend LinkedList<T>;

    public:
        ListNode() : _next(nullptr), _prev(nullptr) {}
        ListNode(const ListNode& node) : _next(node._next), _prev(node._prev), _value(node._value) {}
        ListNode(ListNode&& node) : _next(node._next), _prev(node._prev), _value(node._value) { node._next = node._prev = nullptr; }
        ListNode(const T& value) : _value(value), _next(nullptr), _prev(nullptr) {}
        ListNode(T&& value) : _value(value), _next(nullptr), _prev(nullptr) {}
        ListNode& operator=(const T& value) {
            _value = value;
            return *this;
        }
        ListNode& operator=(const ListNode& element) {
            _value = element._value;
            return *this;
        }
        ListNode& operator=(const ListNode* element) {
            _value = element->_value;
            return *this;
        }
    };
    ListNode *_head, *_tail;
    size_t _size;

public:
    LinkedList() : _head(nullptr), _tail(nullptr), _size(0) {}
    LinkedList(const LinkedList<T>& list) : _head(nullptr), _tail(nullptr), _size(0) {
        for (size_t i = 0; i < list.size(); i++) {
            add(list[i]);
        }
    }
    LinkedList(LinkedList<T>&& list) : _head(list._head), _tail(list._tail), _size(list._size) {
        list._head = list._tail = nullptr;
        list._size = 0;
    }
    LinkedList& operator=(const LinkedList<T>& list) {
        clear();
        for (size_t i = 0; i < list.size(); i++) {
            add(list[i]);
        }
        return *this;
    }
    ~LinkedList() {
        clear();
        delete _head;
        delete _tail;
        _head = _tail = nullptr;
    }
    size_t size() const { return _size; }
    bool empty() const { return _size == 0; }
    void add(const T& value) {
        ListNode* newElement = new ListNode(value);
        if (_size != 0) {
            newElement->_prev = _tail;
            _tail->_next = newElement;
            _tail = newElement;
        } else {
            _head = _tail = newElement;
        }
        ++_size;
    }
    void add(size_t index, const T& value) {
        if (_size == 0) {
            add(value);
            return;
        }
        if (index >= 0 && index < _size) {
            ListNode* node = _head;
            for (size_t i = 0; i < index; ++i) node = node->_next;
            ListNode* newElement = new ListNode(value);
            newElement->_next = node;
            newElement->_prev = node->_prev;
            if (node->_prev != nullptr) node->_prev->_next = newElement;
            node->_prev = newElement;
            if (index == 0) _head = newElement;
            if (index == _size - 1) _tail = newElement->_next;
            ++_size;
        } else
            throw std::out_of_range("LinkedList :: add(index, value)");
    }
    T remove(size_t index) {
        if (index >= 0 && index < _size) {
            ListNode* node = _head;
            for (size_t i = 0; i < index; ++i) node = node->_next;
            if (node->_prev != nullptr) node->_prev->_next = node->_next;
            if (node->_next != nullptr) node->_next->_prev = node->_prev;
            if (_tail == node && node->_prev != nullptr) _tail = node->_prev;
            if (_head == node && node->_next != nullptr) _head = node->_next;
            if (_head == _tail && _tail == node) _head = _tail = nullptr;
            node->_next = nullptr;
            node->_prev = nullptr;
            --_size;
            return node->_value;
        } else
            throw std::out_of_range("LinkedList :: remove(index)");
    }
    void clear() {
        if (_size != 0) {
            ListNode* node = _tail;
            while (node != nullptr) {
                if (node->_next != nullptr) {
                    delete node->_next;
                    node->_next = nullptr;
                }
                if (node->_prev != nullptr)
                    node = node->_prev;
                else {
                    delete node;
                    node = nullptr;
                }
            }
            _head = _tail = nullptr;
            _size = 0;
        }
    }
    T get(size_t index) const {
        if (index >= 0 && index < _size) {
            ListNode* node = _head;
            for (size_t i = 0; i < index; ++i) node = node->_next;
            return node->_value;
        } else
            throw std::out_of_range("LinkedList :: get(index)");
    }
    T set(size_t index, const T& value) {
        if (index >= 0 && index < _size) {
            ListNode* node = _head;
            for (size_t i = 0; i < index; ++i) node = node->_next;
            T tmp = node->_value;
            node->_value = value;
            return tmp;
        } else
            throw std::out_of_range("LinkedList :: set(index, value)");
    }
    bool swap(size_t index1, size_t index2) {
        if (index1 >= 0 && index1 < _size && index2 >= 0 && index2 < _size) {
            ListNode *node1, *node2;
            {
                ListNode* node = _head;
                for (size_t i = 0; i < index1; ++i) node = node->_next;
                node1 = node;
            }
            {
                ListNode* node = _head;
                for (size_t i = 0; i < index2; ++i) node = node->_next;
                node2 = node;
            }
            T tmp = node1->_value;
            node1->_value = node2->_value;
            node2->_value = tmp;
            return true;
        } else
            return false;
    }
    T& operator[](size_t index) const {
        if (index >= 0 && index < _size) {
            ListNode* node = _head;
            for (size_t i = 0; i < index; ++i) node = node->_next;
            return node->_value;
        } else
            throw std::out_of_range("LinkedList :: operator [index]");
    }
};

class FONT_PORT String {
public:
    String();
    String(const char* str, size_t size = 0);
    String(const String& str);
    ~String();
    String& operator=(const String& str);
    bool operator==(const char* str);
    bool operator==(const String& str);
    const char* data() const;
    const size_t size() const;

private:
    char* m_data;
    size_t m_size;
};

struct FONT_PORT SystemFontFaceInfo {
    long lfHeight;
    long lfWidth;
    long lfEscapement;
    long lfOrientation;
    long lfWeight;
    unsigned char lfItalic;
    unsigned char lfUnderline;
    unsigned char lfStrikeOut;
    unsigned char lfCharSet;
    unsigned char lfOutPrecision;
    unsigned char lfClipPrecision;
    unsigned char lfQuality;
    unsigned char lfPitchAndFamily;
    char lfFaceName[32];
};

template class FONT_PORT LinkedList<SystemFontFaceInfo>;
struct FONT_PORT SystemFontInfo {
    String name;
    LinkedList<SystemFontFaceInfo> info;
};

template class FONT_PORT LinkedList<SystemFontInfo>;
FONT_PORT LinkedList<SystemFontInfo>& GetSystemFonts(bool refresh = false);

enum class FONT_PORT GlyphErrorCode {
    Success = 0,
    InvalidGlyph,
    NoBitmapData,
};

struct FONT_PORT GlyphBitmapInfo {
    GlyphErrorCode error_code = GlyphErrorCode::Success;
    void* data = 0;
    int bearing_x = 0;
    int bearing_y = 0;
    int advance = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    bool emoji = false;
};

FONT_PORT void* CreateTTFFont(const String& name, uint32_t size);
FONT_PORT void DestroyTTFFont(void* font);
FONT_PORT String LoadTTFFont(void* ttf_data, uint32_t size);

FONT_PORT GlyphBitmapInfo GetDefaultGlyph();

template class FONT_PORT LinkedList<GlyphBitmapInfo>;
FONT_PORT LinkedList<GlyphBitmapInfo> GetGlyphBitmapInfo(void* font, uint32_t char_code);
}  // namespace Font