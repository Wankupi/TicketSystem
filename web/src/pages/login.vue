<script setup lang="ts">
import { ref, inject, computed } from "vue";
import { backend_key, current_user_key } from "@/keys";
import { ElMessage } from 'element-plus';

const backend = inject(backend_key) as WebSocket;
const username = ref("");
const password = ref("");
const current_user = inject(current_user_key)!;
const validate = computed(() => { return current_user.value.length == 0 && username.value.length > 0 && password.value.length > 0; });

interface LogInRequest {
	username: string
};

let history = new Map<string, LogInRequest>();

function send_request_add_user() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, { username: username.value });
	backend.send(`[${timestamp}] login -u ${username.value} -p ${password.value}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "0") {
		current_user.value = history.get(timestamp)!.username;
		ElMessage.success(`login success.`);
	}
	else {
		ElMessage.error(`login failed.`);
	}
});


</script>
<template>
	<div class="center">
		<div>
			<div>用户名</div><el-input v-model="username" minlength="1" maxlength="20" :show-word-limit=true></el-input>
		</div>
		<div>
			<div>密码</div><el-input v-model="password" type="password" :show-password=true minlength="1" maxlength="30"
				:show-word-limit=true></el-input>
		</div>
		<el-button type="primary" plain class="my-button" @click="send_request_add_user"
			:disabled="!validate">登陆</el-button>
	</div>
</template>

<style>
.center {
	width: 30em;
	margin: 0 auto;
}
</style>
