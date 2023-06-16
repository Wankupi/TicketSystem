<script setup lang="ts">
import { PageState, backend_key, current_user_key, pageState_key } from '@/keys';

import { ElMessage } from 'element-plus';
import { inject } from 'vue';

let current_user = inject(current_user_key)!;
let backend = inject(backend_key)!;
let pageState = inject(pageState_key);

let history = new Set<string>;

function send_request_logout() {
	if (!current_user.value) return;
	let timestamp = (new Date()).toISOString();
	history.add(timestamp);
	backend.send(`[${timestamp}] logout -u ${current_user.value}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "0") {
		current_user.value = "";
		ElMessage.success(`logout success`);
	}
	else {
		ElMessage.error(`logout failed.`);
	}
});

</script>

<template>
	<div>{{ current_user }}</div>
	<el-button @click="current_user ? send_request_logout() : pageState = PageState.login">{{ current_user.length ? "退出登录" :
		"登录" }}</el-button>
</template>
