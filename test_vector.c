#include "test.h"
#include "string.h"
#include "token_vector.h"
#include "nierr.h"

TEST_SUITE_START(TokenVectorTests);

// newVector()
Vector* vec = newTokenVector();
SHOULD_EQUAL("newVector() Size", vec->size, 0);
SHOULD_BE_GRT("newVector() Capacity", vec->capacity, 0)
SHOULD_EQUAL("newVector() End", vec->end, vec->data);
clearError();

// vectorPushDefault()
vectorPushDefaultToken(vec);
SHOULD_EQUAL("vectorPushDefault() Size", vectorSize(vec), 1);
SHOULD_EQUAL("vectorPushDefault() Data", ((Token*)vec->data)->type, STT_Empty);
SHOULD_EQUAL("vectorPushDefault() End", (vec->end - vec->data) / vec->itemSize, 1);
clearError();

// vectorSize()
SHOULD_EQUAL("vectorSize() Size", vectorSize(vec), vec->size);
clearError();

// vectorCapacity()
SHOULD_EQUAL("vectorCapacity() Capacity", vectorCapacity(vec), vec->capacity);
clearError();

// vectorBack()
((Token*)vectorBack(vec))->type = STT_Comma;
SHOULD_EQUAL("vectorBack() Data", ((Token*)vec->data)->type, STT_Comma);
clearError();

// vectorFront()
((Token*)vectorFront(vec))->type = STT_Less;
SHOULD_EQUAL("vectorFront() Data", ((Token*)vec->data)->type, STT_Less);
clearError();

// vectorBegin()
SHOULD_EQUAL("vectorBegin() Data", vectorBegin(vec), vec->data);
clearError();

// vectorPush()
Token* token = newToken();
token->type = STT_Variable;
initStringS(&token->str, "$a", 2);
vectorPushToken(vec, token);
freeToken(&token);
SHOULD_EQUAL("vectorPush() Size", vectorSize(vec), 2);
SHOULD_EQUAL("vectorPush() Data", ((Token*)vectorBack(vec))->type, STT_Variable);
SHOULD_EQUAL_STR("vectorPush() Data", ((Token*)vectorBack(vec))->str.data, "$a");
clearError();

// vectorAt()
SHOULD_EQUAL("vectorAt() Data", ((Token*)vectorAt(vec, 1))->type, STT_Variable);
clearError();

// vectorPop()
vectorPopToken(vec);
SHOULD_EQUAL("vectorPop() Size", vectorSize(vec), 1);
SHOULD_EQUAL("vectorPop() End", (vec->end - vec->data) / vec->itemSize, 1);
clearError();

// vectorReserve()
vectorReserve(vec, 60);
SHOULD_EQUAL("vectorReserve() Capacity", vectorCapacity(vec), 60);
clearError();

// vectorResize()
vectorResizeToken(vec, 120);
SHOULD_EQUAL("vectorResize() Size", vectorSize(vec), 120);
SHOULD_EQUAL("vectorResize() End", (vec->end - vec->data) / vec->itemSize, 120);
SHOULD_EQUAL("vectorResize() Capacity", vectorCapacity(vec), 120);
clearError();

// vectorResize()
vectorResizeToken(vec, 20);
SHOULD_EQUAL("stringResize() Size", vectorSize(vec), 20);
//SHOULD_EQUAL("stringResize() Capacity", vectorCapacity(vec), 40);
clearError();

// vectorShrinkToFit()
vectorShrinkToFit(vec);
SHOULD_EQUAL("vectorShrinkToFit() Capacity", vectorCapacity(vec), 20);
clearError();

// vectorEnd()
SHOULD_EQUAL("vectorEnd() End", vectorEnd(vec), vec->end);
SHOULD_EQUAL("vectorEnd() End", vectorEnd(vec), vec->data + vec->size * vec->itemSize);
clearError();

// vectorStep()
uint8_t i = 0;
for(uint8_t* it = vectorBegin(vec); it != vectorEnd(vec); it += vectorStep(vec))
    i++;
SHOULD_EQUAL("vectorStep() Count", i, 20);
clearError();

// vectorClear()
vectorClearToken(vec);
SHOULD_EQUAL("vectorClear() Size", vectorSize(vec), 0);
SHOULD_EQUAL("vectorClear() Capacity", vectorCapacity(vec), 20);
SHOULD_EQUAL("vectorClear() End", vectorEnd(vec), vectorBegin(vec));
clearError();

// freeVector()
freeTokenVector(&vec);
SHOULD_EQUAL("freeVector() Pointer", vec, NULL);
clearError();

TEST_SUITE_END
