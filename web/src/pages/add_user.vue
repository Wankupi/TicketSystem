<script setup lang="ts">
import { backend_key, current_user_key } from "@/keys";
import { ref, inject, provide } from "vue";
import type { Ref } from "vue";
import { computed } from "@vue/reactivity";
import { ElMessage } from 'element-plus';

const backend = inject(backend_key) as WebSocket;
const username = ref("");
const mailAddr = ref("");
const password = ref("");
const name = ref("");
const privilege = ref(1);
const current_user = inject(current_user_key) as Ref<string>;

const validate = computed(() => {
	if (!(username.value.length && mailAddr.value.length && name.value.length && password.value.length))
		return false;
	if (privilege.value < 1 || privilege.value > 10) return false;
	return true;
});

let history = new Map<string, string>();

function send_request_add_user() {
	if (!validate.value) return;
	let use_name = current_user.value ? current_user.value : "guest";
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, username.value);
	backend.send(`[${timestamp}] add_user -c ${use_name} -u ${username.value} -p ${password.value} -n ${name.value} -m ${mailAddr.value} -g ${privilege.value}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "0") {
		ElMessage.success(`add user ${history.get(timestamp)} success.`);
	}
	else {
		ElMessage.error(`add user ${history.get(timestamp)} failed.`);
	}
});

</script>
<template>
	<h2>添加用户</h2>
	<div class="center">
		<div>
			<div>用户名</div><el-input v-model="username" minlength="1" maxlength="20" :show-word-limit=true></el-input>
		</div>
		<div>
			<div>邮箱</div><el-input v-model="mailAddr" minlength="1" maxlength="30" :show-word-limit=true></el-input>
		</div>
		<div>
			<div>真实名称</div><el-input v-model="name" minlength="1" maxlength="5" :show-word-limit=true></el-input>
		</div>
		<div>
			<div>密码</div><el-input v-model="password" type="password" :show-password=true minlength="1" maxlength="30"
				:show-word-limit=true></el-input>
		</div>
		<div>
			<div>权限</div><el-input-number v-model="privilege" :min="1" :max="10" :precision="0"/>
		</div>
		<el-button type="primary" plain class="my-button" @click="send_request_add_user"
			:disabled="!validate">添加</el-button>
	</div>
</template>
