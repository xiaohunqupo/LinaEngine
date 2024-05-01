/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Core/World/Entity.hpp"
#include "Core/World/Component.hpp"
#include "Common/Serialization/StringSerialization.hpp"
#include "Common/Serialization/VectorSerialization.hpp"
#include "Common/Serialization/SetSerialization.hpp"
#include "Common/Data/CommonData.hpp"

namespace Lina
{
	void Entity::AddChild(Entity* e)
	{
		e->RemoveFromParent();
		m_children.push_back(e);
		e->m_parentID = m_id;
		e->m_parent	  = this;
	}

	void Entity::RemoveChild(Entity* e)
	{
		m_children.erase(linatl::find_if(m_children.begin(), m_children.end(), [e](Entity* c) { return e == c; }));
		e->m_parentID = ENTITY_NULL;
		e->m_parent	  = nullptr;
	}

	void Entity::RemoveFromParent()
	{
		if (m_parentID != ENTITY_NULL)
			m_parent->RemoveChild(this);
	}

	void Entity::SetTransformation(Matrix4& mat, bool omitScale)
	{
		Vector3	   loc;
		Quaternion rot;
		Vector3	   scale;
		mat.Decompose(loc, rot, scale);
		SetPosition(loc);
		SetRotation(rot);

		if (!omitScale)
			SetScale(scale);
	}

	void Entity::SetLocalTransformation(Matrix4& mat, bool omitScale)
	{
		Vector3	   loc;
		Quaternion rot;
		Vector3	   scale;
		mat.Decompose(loc, rot, scale);
		SetLocalPosition(loc);
		SetLocalRotation(rot);

		if (!omitScale)
			SetLocalScale(scale);
	}

	void Entity::AddRotation(const Vector3& angles)
	{
		SetRotationAngles(GetRotationAngles() + angles);
	}

	void Entity::AddLocalRotation(const Vector3& angles)
	{
		SetLocalRotationAngles(GetLocalRotationAngles() + angles);
	}

	void Entity::AddPosition(const Vector3& loc)
	{
		SetPosition(GetPosition() + loc);
	}

	void Entity::AddLocalPosition(const Vector3& loc)
	{
		SetLocalPosition(GetLocalPosition() + loc);
	}

	Transformation Entity::GetInterpolated(float interpolation)
	{
		Transformation t;
		t.m_position = Vector3::Lerp(m_prevTransform.m_position, m_transform.m_position, interpolation);
		t.m_scale	 = Vector3::Lerp(m_prevTransform.m_scale, m_transform.m_scale, interpolation);
		t.m_rotation = Quaternion::Euler(Vector3::Lerp(m_prevTransform.m_rotationAngles, m_transform.m_rotation.GetEuler(), interpolation));
		return t;
	}

	bool Entity::HasChildInTree(Entity* other) const
	{
		auto it = linatl::find_if(m_children.begin(), m_children.end(), [other](Entity* e) { return e == other || e->HasChildInTree(other); });
		return it != m_children.end();
	}

	void Entity::SetLocalPosition(const Vector3& loc)
	{
		m_transform.m_localPosition = loc;
		UpdateGlobalPosition();

		for (auto child : m_children)
			child->UpdateGlobalPosition();
	}
	void Entity::SetPosition(const Vector3& loc)
	{
		m_mask.Set(EntityMask::NeedsVisualUpdate);

		m_transform.m_position = loc;
		UpdateLocalPosition();

		for (auto child : m_children)
			child->UpdateGlobalPosition();
	}

	void Entity::SetLocalRotation(const Quaternion& rot, bool isThisPivot)
	{
		m_transform.m_localRotation		  = rot;
		m_transform.m_localRotationAngles = rot.GetEuler();
		UpdateGlobalRotation();

		for (auto child : m_children)
		{
			child->UpdateGlobalRotation();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::SetLocalRotationAngles(const Vector3& angles, bool isThisPivot)
	{
		m_transform.m_localRotationAngles = angles;
		const Vector3 vang				  = glm::radians(static_cast<glm::vec3>(angles));
		m_transform.m_localRotation		  = Quaternion::FromVector(vang);
		UpdateGlobalRotation();

		for (auto child : m_children)
		{
			child->UpdateGlobalRotation();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::SetRotation(const Quaternion& rot, bool isThisPivot)
	{
		m_mask.Set(EntityMask::NeedsVisualUpdate);

		m_transform.m_rotation		 = rot;
		m_transform.m_rotationAngles = rot.GetEuler();
		UpdateLocalRotation();

		for (auto child : m_children)
		{
			child->UpdateGlobalRotation();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::SetRotationAngles(const Vector3& angles, bool isThisPivot)
	{
		m_transform.m_rotationAngles = angles;
		m_transform.m_rotation		 = Quaternion::FromVector(glm::radians(static_cast<glm::vec3>(angles)));
		UpdateLocalRotation();

		for (auto child : m_children)
		{
			child->UpdateGlobalRotation();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::SetLocalScale(const Vector3& scale, bool isThisPivot)
	{
		m_transform.m_localScale = scale;
		UpdateGlobalScale();

		for (auto child : m_children)
		{
			child->UpdateGlobalScale();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::SetScale(const Vector3& scale, bool isThisPivot)
	{
		m_mask.Set(EntityMask::NeedsVisualUpdate);

		m_transform.m_scale = scale;
		UpdateLocalScale();

		for (auto child : m_children)
		{
			child->UpdateGlobalScale();

			if (isThisPivot)
				child->UpdateGlobalPosition();
		}
	}

	void Entity::UpdateGlobalPosition()
	{
		if (m_parent == nullptr)
			m_transform.m_position = m_transform.m_localPosition;
		else
		{
			Matrix4 global		   = m_parent->m_transform.ToMatrix() * m_transform.ToLocalMatrix();
			Vector3 translation	   = global.GetTranslation();
			m_transform.m_position = translation;
		}

		for (auto child : m_children)
		{
			child->UpdateGlobalPosition();
		}
	}

	void Entity::UpdateLocalPosition()
	{
		if (m_parent == nullptr)
			m_transform.m_localPosition = m_transform.m_position;
		else
		{
			Matrix4 global				= m_parent->m_transform.ToMatrix().Inverse() * m_transform.ToMatrix();
			m_transform.m_localPosition = global.GetTranslation();
		}
	}

	void Entity::UpdateGlobalScale()
	{
		if (m_parent == nullptr)
			m_transform.m_scale = m_transform.m_localScale;
		else
		{
			Matrix4 global = Matrix4::Scale(m_parent->m_transform.m_scale) * Matrix4::Scale(m_transform.m_localScale);
			Vector3 scale  = global.GetScale();

			m_transform.m_scale = scale;
		}

		for (auto child : m_children)
		{
			child->UpdateGlobalScale();
		}
	}

	void Entity::UpdateGlobalRotation()
	{
		if (m_parent == nullptr)
		{
			m_transform.m_rotation		 = m_transform.m_localRotation;
			m_transform.m_rotationAngles = m_transform.m_localRotationAngles;
		}
		else
		{
			Matrix4	   global = Matrix4::InitRotation(m_parent->m_transform.m_rotation) * m_transform.ToLocalMatrix();
			Quaternion targetRot;
			Vector3	   s = Vector3(), p = Vector3();
			global.Decompose(p, targetRot, s);
			m_transform.m_rotation		 = targetRot;
			m_transform.m_rotationAngles = m_transform.m_rotation.GetEuler();
		}

		for (auto child : m_children)
		{
			child->UpdateGlobalRotation();
		}
	}

	void Entity::UpdateLocalScale()
	{

		if (m_parent == nullptr)
			m_transform.m_localScale = m_transform.m_scale;
		else
		{
			Matrix4 global			 = Matrix4::Scale(m_parent->m_transform.m_scale).Inverse() * Matrix4::Scale(m_transform.m_scale);
			m_transform.m_localScale = global.GetScale();
		}
	}

	void Entity::SaveToStream(OStream& stream)
	{
		stream << m_id << m_parentID;
		m_mask.SaveToStream(stream);
		m_transform.SaveToStream(stream);
		StringSerialization::SaveToStream(stream, m_name);

		m_childrenIDsForLoad.clear();
		m_childrenIDsForLoad.reserve(m_children.size());

		for (auto c : m_children)
			m_childrenIDsForLoad.push_back(c->GetID());

		VectorSerialization::SaveToStream_PT(stream, m_childrenIDsForLoad);
	}

	void Entity::LoadFromStream(IStream& stream)
	{
		stream >> m_id >> m_parentID;
		m_mask.LoadFromStream(stream);
		m_transform.LoadFromStream(stream);
		StringSerialization::LoadFromStream(stream, m_name);

		m_childrenIDsForLoad.clear();
		VectorSerialization::LoadFromStream_PT<uint32>(stream, m_childrenIDsForLoad);
	}

	void Entity::UpdateLocalRotation()
	{
		if (m_parent == nullptr)
		{
			m_transform.m_localRotation		  = m_transform.m_rotation;
			m_transform.m_localRotationAngles = m_transform.m_rotationAngles;
		}
		else
		{
			Matrix4 global = Matrix4::InitRotation(m_parent->m_transform.m_rotation).Inverse() * m_transform.ToMatrix();
			Vector3 s = Vector3(), p = Vector3();
			global.Decompose(s, m_transform.m_localRotation, p);
			m_transform.m_localRotationAngles = m_transform.m_localRotation.GetEuler();
		}
	}

	void Entity::SetVisible(bool visible)
	{
		if (visible)
			m_mask.Set(EntityMask::Visible);
		else
			m_mask.Remove(EntityMask::Visible);

		for (auto c : m_children)
			c->SetVisible(visible);
	}

	void Entity::SetStatic(bool isStatic)
	{
		if (isStatic)
			m_mask.Set(EntityMask::Static);
		else
			m_mask.Remove(EntityMask::Static);

		for (auto c : m_children)
			c->SetStatic(isStatic);
	}
} // namespace Lina
