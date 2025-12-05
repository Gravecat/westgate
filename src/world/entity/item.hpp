// world/entity/item.hpp -- Derived from Entity, an Item is something that can be picked up and carried. This covers equipment, treasure, and many other things.

/*
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: Copyright (c) 2025 Raine "Gravecat" Simmons <gc@gravecat.com>
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 */

#pragma once
#include "core/pch.hpp" // precompiled header

#include "world/entity/entity.hpp"

namespace westgate {

class Item : public Entity {
public:
                        Item() = delete;    // No default constructor; use nullptr on the constructor below.
                        Item(FileReader* file); // Creates a blank Item, then loads its data from a FileReader.
    virtual             ~Item() = default;  // Virtual destructor, in case Item is used by a further derived class in the future.
    virtual void        save(FileWriter* file) override;    // Saves this Item to a save game file.
    virtual EntityType  type() const override { return EntityType::ITEM; }  // Self-identifies this Entity's derived class.

private:
};

}   // namespace westgate
