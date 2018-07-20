/**
 * @file
 * @brief Functions and classes to create/display GUI elements.
 *
 * Example: display a simple dialog with a label
 * @code{cpp}
 * GUIDialog dialog(
 *     GUIDialog::Height25, GUIDialog::AlignTop,
 *     "Dialog Title",
 *     GUIDialog::KeyboardStateABC
 * );
 * 
 * GUILabel label(
 *     dialog.GetLeftX() + 10,
 *     dialog.GetTopY() + 10,
 *     "Label Text"
 * );
 * dialog.AddElement(label);
 * 
 * dialog.ShowDialog();
 * @endcode
 */

#pragma once
#include <stdint.h>

// No point in documenting these macros. If people are trying to use them,
// they're doing something weird and I don't want to help them :)
/// @cond INTERNAL
/**
 * Create @p n fake vtable function entries.
 * 
 * The parameter @p x is required as the fake entry needs a name. It is
 * recommended to simply increment from 0 as fake entries are required.
 * 
 * @param n The number of fake vtable entries to create.
 * @param x An integer, unique within the vtable.
 */
#define VTABLE_FAKE_ENTRY(n, x) uint32_t fakeentry##x[n * 3]

/**
 * Create a vtable function entry.
 * 
 * Makes entries for the offset (suffix @c _Offset)
 * (added to @c self when the function is called), an unused padding entry, and
 * an entry for the function pointer.
 * 
 * @param return_type The return type of the function.
 * @param name The name of the function.
 * @param ... The arguments to the function (type and name).
 */
#define VTABLE_ENTRY(return_type, name, ...) int32_t name##_Offset; \
                                             uint32_t name##_Unused; \
                                             return_type (*name)(__VA_ARGS__)

/**
 * Call a function within an external vtable.
 * 
 * @param vtable The vtable containing the function.
 * @param name The name of the function to call.
 * @param self The pointer to the object to call the function on.
 * @param ... The arguments to the function.
 */
#define VTABLE_CALL_EXT(self, vtable, name, ...) (vtable)->name( \
	reinterpret_cast<decltype(self)>( \
		reinterpret_cast<uint8_t *>(self) + (vtable)->name##_Offset \
	), \
	## __VA_ARGS__ \
)

/**
 * Call a function within a vtable.
 * 
 * @param vtable The name of the vtable inside @p self.
 * @param name The name of the function to call.
 * @param self The pointer to the object to call the function on.
 * @param ... The arguments to the function.
 */
#define VTABLE_CALL(self, vtable, name, ...) VTABLE_CALL_EXT( \
	self, self->vtable, name, ## __VA_ARGS__ \
)
/// @endcond

const int BUTTON_OK = 1 << 5;
const int BUTTON_YES = 1 << 6;
const int BUTTON_NO = 1 << 7;
const int BUTTON_ABORT = 1 << 8;
const int BUTTON_RETRY = 1 << 9;
const int BUTTON_CANCEL = 1 << 10;

/**
 * Wraps an internal class from the fx-CP400.
 */
class Wrapped {
public:
	/**
	 * Returns a pointer to the internal class.
	 * 
	 * @tparam T The type of the pointer to return. Can be @c void.
	 * @return A pointer to the internal class.
	 */
	template <typename T>
	T *GetWrapped() {
		return static_cast<T *>(m_wrapped);
	}

protected:
	Wrapped() = default;
	~Wrapped() = default;

	// Since we use the trick with the vtable to overide functions (@c .me), we
	// can't let the object's address change. Prevent that by deleting the copy
	// constructor and assignment operator.
	Wrapped(Wrapped const &) = delete;
	void operator=(Wrapped const &) = delete;

	/// A pointer to the wrapped class.
	void *m_wrapped;
};

/**
 * Utility class which is the parent of all GUI element classes.
 */
class GUIElement : public Wrapped {};

// pre-declare the class, since a pointer to it is placed in the vtable
class GUIDialog;

/// @private
struct GUIDialog_Wrapped_VTable {
	/**
	 * Stores the address of the @ref GUIDialog object this vtable belongs to.
	 * Should be set by the constructor of @ref GUIDialog.
	 * 
	 * In the original vtable, the first dword is zero. Since it appears to
	 * never be accessed, and the vtables in the firmware are stored in ROM (i.e
	 * even if the firmware wanted to change the value, it couldn't) we're safe
	 * to use it for our own purposes.
	 * 
	 * That purpose is to link the vtable to an instance of the @ref GUIDialog.
	 * Since we can't populate the vtable with instance function pointers, we
	 * have to use a static function. Without this entry, the static function
	 * would have no idea which @ref GUIDialog instance the function call was
	 * associated with.
	 */
	GUIDialog *me;
	uint32_t fakeentrypadding[2];

	VTABLE_FAKE_ENTRY(1, 0);
	
	VTABLE_ENTRY(
		int, OnEvent,
		struct GUIDialog_Wrapped *dialog, struct GUIDialog_OnEvent_Data *event
	);
	VTABLE_FAKE_ENTRY(1, 1);

	// unknown0 - always pass 0
	VTABLE_ENTRY(
		void, AddElement,
		struct GUIDialog_Wrapped *dialog, void *element, int unknown0
	);

	VTABLE_FAKE_ENTRY(4, 2);

	VTABLE_ENTRY(
		void, Refresh,
		struct GUIDialog_Wrapped *dialog
	);

	VTABLE_FAKE_ENTRY(23, 3);

	VTABLE_ENTRY(
		void, ShowDialog,
		struct GUIDialog_Wrapped *dialog
	);

	VTABLE_FAKE_ENTRY(20, 4);
};

/// @private
struct GUIDialog_Wrapped {
	uint8_t unknown0[0x10];

	// refer to accessors in GUIDialog class for documentation
	uint16_t leftX;
	uint16_t topY;
	uint16_t rightX;
	uint16_t bottomY;

	uint8_t unknown1[0x34];

	struct GUIDialog_Wrapped_VTable *vtable;

	uint8_t unknown2[0x58];
};
static_assert(sizeof(struct GUIDialog_Wrapped) == 0xA8);

struct GUIDialog_OnEvent_Data {
	uint16_t type;
	uint16_t unknown0;

	/// The pointer to the internal GUI element class the event refers to. 
	void *element;
};

class GUIDialog : public Wrapped {
public:
	enum Height : int {
		Height25 = 0,
		Height55 = 1,
		Height75 = 2,
		Height95 = 3,
		Height35 = 4,
		Height60 = 5
	};

	enum Alignment : int {
		AlignTop = 0,
		AlignCenter = 1,
		AlignBottom = 2
	};

	enum KeyboardState : int {
		KeyboardStateNone = 0, // 2 gives same effect
		KeyboardStateMath1 = 1, // 3 gives same effect
		KeyboardStateMath2 = 4,
		KeyboardStateMath3 = 5,
		KeyboardStateTrig = 6,
		KeyboardStateVar = 7,
		KeyboardStateABC = 8,
		KeyboardStateCatalog = 9,
		KeyboardStateAdvance = 10,
		KeyboardStateNumber = 11
	};

	GUIDialog(
		enum Height height, enum Alignment alignment,
		const char* title,
		enum KeyboardState keyboard
	);

	virtual int OnEvent(struct GUIDialog_Wrapped *dialog, struct GUIDialog_OnEvent_Data *event);
	
	uint16_t GetLeftX();
	uint16_t GetTopY();
	uint16_t GetRightX();
	uint16_t GetBottomY();

	void AddElement(GUIElement &element);
	void Refresh();
	void ShowDialog();

private:
	struct GUIDialog_Wrapped_VTable *m_oldVTable;
	struct GUIDialog_Wrapped_VTable m_vtable;

	static int OnEvent_Wrap(struct GUIDialog_Wrapped *dialog, struct GUIDialog_OnEvent_Data *event);
};

class GUIButton : public GUIElement {
public:
	enum Flag : int {
		/// Allows the button to be pressed.
		FlagEnabled = 1 << 15
	};

	GUIButton(
		uint16_t leftX, uint16_t topY, uint16_t rightX, uint16_t bottomY,
		const char *text,
		uint16_t eventType
	);
	GUIButton(
		uint16_t leftX, uint16_t topY, uint16_t rightX, uint16_t bottomY,
		const char *text,
		uint16_t eventType, int flags
	);

	/**
	 * Returns the type which will be reported in the event data passed to a
	 * dialog's OnEvent from an @c eventType passed into the constructor for a
	 * @ref GUIButton.
	 * 
	 * @param eventType The @c eventType value passed into the button's
	 * constructor.
	 * @return The type reported in the event data for the specified
	 * @p eventType.
	 */
	static constexpr uint16_t GetEventType(uint16_t eventType) {
		return ((eventType + 8) << 4) | (1 << 3);
	}
};

// Predef
class GUIDropDownMenuItem;

struct GUIDropDownMenu_Wrapped_VTable {
	VTABLE_FAKE_ENTRY(5, 0);

	VTABLE_ENTRY(
		void, AddMenuItem,
		void *dropDownMenu, void *dropDownMenuItem, uint32_t unk0
	);

	VTABLE_FAKE_ENTRY(28, 1);

	VTABLE_ENTRY(
		void, SetScrollBarVisibility,
		void *dropDownMenu, uint32_t visibility
	);
};

struct GUIDropDownMenu_Wrapped {
	uint8_t unknown0[0x4C];

	struct GUIDropDownMenu_Wrapped_VTable *vtable;
};

class GUIDropDownMenu : public GUIElement {
public:
	enum ScrollBarVisibility : uint32_t {
		ScrollBarHidden = 0,
		ScrollBarAlwaysVisible = 1,
		ScrollBarVisibleWhenRequired = 2
	};

	GUIDropDownMenu(
		uint16_t leftX, uint16_t topY, uint16_t rightX, uint16_t bottomY,
		uint16_t eventType
	);

	void SetScrollBarVisibility(ScrollBarVisibility visibility);
	void AddMenuItem(GUIDropDownMenuItem &dropDownMenuItem);
};

class GUIDropDownMenuItem : public GUIElement {
public:
	enum Flag : int {
		FlagTextAlignRight = 1 << 5,
		FlagTextAlignLeft = 1 << 6,
		FlagEnabled = 1 << 15
	};

	GUIDropDownMenuItem(const char *s, int idx, int flags);
};

class GUILabel : public GUIElement {
public:
	enum Flag : int {
		/// Enables displaying the background color of the label.
		FlagBackground = 1 << 0,

		/**
		 * Allows the label to be selected/brought into focus.
		 * 
		 * When the label is selected, the text and background colors switch.
		 */
		FlagSelectable = 1 << 15
	};

	GUILabel(int x, int y, const char *text);
	GUILabel(
		int x, int y,
		const char *text,
		int flags,
		uint16_t *textColor, uint16_t *backgroundColor
	);
	GUILabel(
		int x, int y,
		const char *text,
		int flags,
		uint16_t *textColor, uint16_t *backgroundColor,
		bool showShadow, uint16_t shadowColor
	);
};

class GUIRadioButton : public GUIElement {
public:
	enum Flag : int {
		/// Causes the radio button to be selected by default.
		FlagSelected = 1 << 2,

		/// Makes the radio button interactive.
		FlagEnabled = 1 << 15
	};

	GUIRadioButton(
		int x, int y,
		const char *text,
		int flags
	);
};

/// @private
struct GUITextBox_Wrapped_VTable {
	VTABLE_FAKE_ENTRY(32, 0);

	VTABLE_ENTRY(
		void, SetText,
		struct GUITextBox_Wrapped *textBox, const char *text
	);

	// no need to length match
};

/// @private
struct GUITextBox_Wrapped {
	uint8_t unknown0[0x4C];

	struct GUITextBox_Wrapped_VTable *vtable;

	uint8_t unknown1[0x4];

	const char *text;

	uint8_t unknown2[0x48];
};
static_assert(sizeof(struct GUITextBox_Wrapped) == 0xA0);

class GUITextBox : public GUIElement {
public:
	enum Flag {
		/// Enables drawing the text box's outline and background.
		FlagDrawBox = 1 << 3,

		/// Allows the contents of the text box to be modified.
		FlagEditable = 1 << 8
	};

	GUITextBox(
		int x, int y, int width,
		int maxLength, bool countLengthByBytes
	);
	GUITextBox(
		int x, int y, int width,
		const char *text, int maxLength, bool countLengthByBytes
	);

	const char *GetText();
	void SetText(const char *text);
};

/// @cond INTERNAL
extern "C"
void *GUI_CreateButton(
	void *button,
	uint16_t bounds[4],
	const char *text,
	uint16_t eventType, int unk0, int unk1
);

extern "C"
void *GUI_CreateDropDownMenu(
	void *dropDownMenu,
	uint16_t bounds[4],
	uint16_t eventType, int flags1
);

extern "C"
void *GUI_CreateDropDownMenuItem(
	void *dropDownMenuItem,
	int unk0, int unk1,
	const char *text,
	int index, int flags,
	int unk2
);

extern "C"
struct GUIDialog_Wrapped *GUI_CreateDialog(
	void *dialog,
	int height, int alignment,
	const char *title,
	int unknown2, int unknown3,
	int keyboard
);

extern "C"
void *GUI_CreateLabel(
	void *label,
	int x, int y,
	const char *text,
	int unknown0,
	int flags,
	void *font,
	uint16_t *textColor, uint16_t *backgroundColor,
	bool showShadow, uint16_t shadowColor,
	int unknown1
);

extern "C"
void *GUI_CreateRadioButton(
	void *radioButton,
	int x, int y,
	const char *text,
	int unknown0,
	int flags,
	void *font,
	int unknown2
);

extern "C"
struct GUITextBox_Wrapped *GUI_CreateTextBox(
	void *textBox,
	int x, int y, int width,
	const char *text,
	int unknown0,
	int flags,
	int maxLength, bool countLengthByBytes
);
/// @endcond

/**
 * Displays a message box with the specified title and content, retrieved
 * through their ID in the string table.
 *
 * @param unknown An unknown parameter.
 * @param titleStringID The ID of the string to use for the message box's title.
 * @param contentStringID The ID of the string to use for the message box's
 * content.
 */
extern "C"
void GUI_DisplayMessageBox(int unknown, int titleStringID, int contentStringID);

/**
 * Displays a message box with the specified title and content. A prefix to the
 * content may be specified, which is displayed before the main content.
 * 
 * The buttons to be displayed can be set using the @p buttons bitfield.
 * Possible values are defined by macros beginning with @c BUTTON_, which can be
 * bitwise OR'd together to display multiple buttons. A maximum of 3 buttons can
 * be specified.
 * 
 * The true usage of the 4th bit of the @p buttons bitfield is unknown, however
 * it has been observed to shrink the black box which forms the title bar of the
 * message box.
 * 
 * Note: if no buttons are specified, and the close button is disabled, it is
 * impossible to exit the message box.
 * 
 * @param unknown An unknown parameter.
 * @param[in] titleString A string to use for the message box's title.
 * @param[in] contentPrefix A string to prefix the content with.
 * @param[in] contentString A string to use for the message box's content.
 * @param buttons A bitfield specifying which buttons to show.
 * @param disableCloseButton Set to @c true to disable the close button.
 * @return An unknown GUI struct.
 */
extern "C"
void *GUI_DisplayMessageBox_Internal(
	int unknown,
	const char *titleString,
	const char *contentPrefix, const char *contentString,
	int buttons, bool disableCloseButton
);
