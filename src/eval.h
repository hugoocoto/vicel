/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */

#ifndef EVAL_H_
#define EVAL_H_

#include "cellmap.h"
#include "formula.h"

Value eval_formula(Formula f);
Value eval_expr(Expr *e);


/* basic operations on Values */
Value vadd(Value a, Value b);
Value vsub(Value a, Value b);
Value vdiv(Value a, Value b);
Value vmul(Value a, Value b);
Value vpow(Value a, Value b);


#endif //! EVAL_H_
